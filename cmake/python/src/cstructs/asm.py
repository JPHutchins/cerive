"""Assembly canonicalization and diffing."""

import difflib
import re
from functools import reduce
from typing import NamedTuple

_COMMENT = re.compile(r"@.*$")
_DROP = re.compile(
    r"^\s*\.(cfi_|loc\b|file\b|ident\b|size\b|type\b|globl?\b|global\b|weak\b"
    r"|syntax\b|thumb|arm\b|arch\b|fpu\b|cpu\b|eabi_attribute\b|fnstart\b|fnend\b"
    r"|save\b|pad\b|setfp\b|movsp\b|code\b|align\b|p2align\b|balign\b|ltorg\b|pool\b"
    r"|section\b|text\b|data\b)"
)
_FUNC = re.compile(r"^\s*\.type\s+(\w+),\s*%function")
_THUMB_FUNC = re.compile(r"^\s*\.thumb_func\s*$")
_THUMB_LABEL = re.compile(r"^[.\w]+:$")
_LOCAL = re.compile(r"\.L\w+")


class _Scan(NamedTuple):
    funcs: tuple[tuple[str, str], ...] = ()
    pending: str | None = None
    current: str | None = None
    body: tuple[str, ...] = ()
    thumb_pending: bool = False


def _ends(current: str, line: str) -> bool:
    return re.match(rf"^\s*\.size\s+{re.escape(current)}\b", line) is not None


def _scan(state: _Scan, line: str) -> _Scan:
    if (head := _FUNC.match(line)) is not None:
        return state._replace(pending=head.group(1))
    if _THUMB_FUNC.match(line):
        return state._replace(thumb_pending=True)
    if state.pending is not None and line.strip() == f"{state.pending}:":
        return state._replace(current=state.pending, body=(), pending=None)
    if state.current is not None and _ends(state.current, line):
        return state._replace(
            funcs=(*state.funcs, (state.current, "\n".join(state.body))), current=None
        )
    if state.current is not None:
        return state._replace(body=(*state.body, line))
    if state.thumb_pending and _THUMB_LABEL.match(line.strip()):
        return state._replace(current=line.strip()[:-1], body=(), thumb_pending=False)
    return state


def split_functions(asm: str) -> dict[str, str]:
    r"""Split assembly into function bodies.

    >>> split_functions("\t.type f, %function\nf:\n\tnop\n\t.size f, .-f\n")["f"].strip()
    'nop'
    >>> split_functions("\t.thumb_func\nf:\n\tnop\n\t.size f, .-f\n")["f"].strip()
    'nop'
    >>> split_functions("\t.type f, %function\nf:\n\tnop\n")
    {}
    """
    return dict(reduce(_scan, asm.splitlines(), _Scan()).funcs)


def _renumbered(text: str) -> str:
    labels = {label: f".L{n}" for n, label in enumerate(dict.fromkeys(_LOCAL.findall(text)))}
    return _LOCAL.sub(lambda m: labels[m.group(0)], text)


def canonical(body: str) -> str:
    r"""Canonicalize a function body for comparison.

    >>> canonical("\tbl\tcerive_buf_remaining\t@ x\n.L7:\n\tbx\tlr")
    'bl\tcerive_buf_remaining\n.L0:\nbx\tlr'
    >>> canonical("\tb\t.L9\n.L3:\n\tb\t.L9\n.L9:")
    'b\t.L0\n.L1:\nb\t.L0\n.L0:'
    """
    return _renumbered(
        "\n".join(
            line.strip()
            for line in (_COMMENT.sub("", raw).rstrip() for raw in body.splitlines())
            if line.strip() and not _DROP.match(line)
        )
    )


def instr_count(canon: str) -> int:
    r"""Instruction count.

    >>> instr_count("mov\tr0, r1\n.L0:\nbx\tlr")
    2
    """
    return sum(
        1 for ln in canon.splitlines() if ln and not ln.endswith(":") and not ln.startswith(".")
    )


def diff_lines(a: str, b: str, a_label: str, b_label: str) -> str:
    """Unified diff."""
    if a == b:
        return ""
    return "\n".join(
        difflib.unified_diff(
            a.splitlines(), b.splitlines(), fromfile=a_label, tofile=b_label, lineterm=""
        )
    )


_SYM = re.compile(r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+[TtWw]\s+(\w+)\s*$")


def parse_syms(nm_output: str) -> dict[str, int]:
    r"""Parse `nm --print-size` output.

    >>> parse_syms("00000000 00000018 T study_eq\n0000abcd t local_no_size")
    {'study_eq': 24}
    >>> parse_syms("00000000 00000004 r table\n00000000 00000004 d names")
    {}
    """
    return {
        m.group(2): int(m.group(1), 16)
        for m in map(_SYM.match, nm_output.splitlines())
        if m is not None
    }
