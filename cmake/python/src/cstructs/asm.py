"""Canonicalize and diff compiler assembly so only real codegen differences show."""

import difflib
import re

_COMMENT = re.compile(r"@.*$")
_DROP = re.compile(
    r"^\s*\.(cfi_|loc\b|file\b|ident\b|size\b|type\b|globl?\b|global\b|weak\b"
    r"|syntax\b|thumb|arm\b|arch\b|fpu\b|cpu\b|eabi_attribute\b|fnstart\b|fnend\b"
    r"|save\b|pad\b|setfp\b|movsp\b|code\b|align\b|p2align\b|section\b|text\b|data\b)"
)
_FUNC = re.compile(r"^\s*\.type\s+(\w+),\s*%function")
_HELPER = re.compile(r"\b(?:derive|hw)_(at|rem)\b")
_LOCAL = re.compile(r"\.L\w+")


def split_functions(asm: str) -> dict[str, str]:
    """Map each `%function` symbol to its raw body (between `name:` and `.size`).

    >>> split_functions("\\t.type f, %function\\nf:\\n\\tnop\\n\\t.size f, .-f\\n")["f"].strip()
    'nop'
    """
    funcs: dict[str, str] = {}
    pending: str | None = None
    current: str | None = None
    body: list[str] = []
    for line in asm.splitlines():
        head = _FUNC.match(line)
        if head is not None:
            pending = head.group(1)
        elif pending is not None and line.strip() == f"{pending}:":
            current, body, pending = pending, [], None
        elif current is not None and re.match(rf"^\s*\.size\s+{re.escape(current)}\b", line):
            funcs[current] = "\n".join(body)
            current = None
        elif current is not None:
            body.append(line)
    return funcs


def canonical(body: str) -> str:
    """Drop noise (comments, CFI/debug/section directives), neutralize helper
    symbol names and renumber local labels -- leaving comparable instructions.

    >>> canonical("\\tbl\\tderive_rem\\t@ x\\n.L7:\\n\\tbx\\tlr")
    'bl\\tH_rem\\n.L0:\\nbx\\tlr'
    """
    kept: list[str] = []
    for raw in body.splitlines():
        line = _COMMENT.sub("", raw).rstrip()
        if not line.strip() or _DROP.match(line):
            continue
        kept.append(_HELPER.sub(r"H_\1", line).strip())
    labels: dict[str, str] = {}
    return _LOCAL.sub(lambda m: labels.setdefault(m.group(0), f".L{len(labels)}"), "\n".join(kept))


def instr_count(canon: str) -> int:
    """Count instruction lines (excluding labels and data directives).

    >>> instr_count("mov\\tr0, r1\\n.L0:\\nbx\\tlr")
    2
    """
    return sum(1 for ln in canon.splitlines() if ln and not ln.endswith(":") and not ln.startswith("."))


def diff_lines(a: str, b: str, a_label: str, b_label: str) -> str:
    """Unified diff between two canonical bodies; empty string when identical."""
    if a == b:
        return ""
    return "\n".join(
        difflib.unified_diff(a.splitlines(), b.splitlines(), fromfile=a_label, tofile=b_label, lineterm="")
    )


_SYM = re.compile(r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+\w\s+(\w+)\s*$")


def parse_syms(nm_output: str) -> dict[str, int]:
    """Parse `nm --print-size` lines into {symbol: byte size}.

    >>> parse_syms("00000000 00000018 T study_eq\\n0000abcd t local_no_size")
    {'study_eq': 24}
    """
    out: dict[str, int] = {}
    for line in nm_output.splitlines():
        m = _SYM.match(line)
        if m is not None:
            out[m.group(2)] = int(m.group(1), 16)
    return out
