"""Evidence report rendering."""

import re
from typing import TYPE_CHECKING, NamedTuple, assert_never

from cstructs.asm import diff_lines, instr_count

if TYPE_CHECKING:
    from collections.abc import Mapping, Sequence


class Key(NamedTuple):
    """A matrix lookup key."""

    impl: str
    cpu: str
    opt: str
    fn: str


class Stem(NamedTuple):
    """A matrix artifact stem.

    >>> str(Stem("cerive", "cortex-m3", "O0"))
    'cerive.cortex-m3.O0'
    """

    impl: str
    cpu: str
    opt: str

    def __str__(self) -> str:
        return f"{self.impl}.{self.cpu}.{self.opt}"


class SizeInfo(NamedTuple):
    """`size` output for one object."""

    text: int
    data: int
    bss: int


_BERKELEY_ROW = re.compile(
    r"^[ \t]*(\d+)[ \t]+(\d+)[ \t]+(\d+)[ \t]+\d+[ \t]+[0-9a-fA-F]+[ \t]+\S", re.MULTILINE
)


def parse_size(output: str) -> SizeInfo | None:
    r"""Parse Berkeley `size` output.

    >>> parse_size("  text  data  bss\n  88  0  4  92  5c  x.o")
    SizeInfo(text=88, data=0, bss=4)
    >>> parse_size("nonsense") is None
    True
    >>> parse_size("error 1 2 3") is None
    True
    >>> parse_size(" 88 0\n 4 92 5c x.o") is None
    True
    """
    row = _BERKELEY_ROW.search(output)
    return SizeInfo(*map(int, row.groups())) if row else None


def _is_helper(fn: str) -> bool:
    return fn.startswith("cerive_")


class Cell(NamedTuple):
    """A comparison cell."""

    fn: str
    cpu: str
    opt: str


class Gap(NamedTuple):
    """An evidence gap."""

    cpu: str
    opt: str
    reason: str


class Report(NamedTuple):
    """A rendered evidence report and its verdict."""

    markdown: str
    failures: tuple[str, ...]


class Absent(NamedTuple):
    """A cell judgement."""


class Missing(NamedTuple):
    """A cell judgement."""


class Identical(NamedTuple):
    """A cell judgement."""


class Divergent(NamedTuple):
    """A cell judgement."""

    mark: str
    rival: bool
    off_baseline: bool
    diff: str


type Judgement = Absent | Missing | Identical | Divergent


def _mark(judgement: Judgement) -> str:
    match judgement:
        case Absent():
            return ""
        case Missing():
            return "∅"
        case Identical():
            return "="
        case Divergent(mark=mark):
            return mark
        case _:
            assert_never(judgement)


def _off_baseline(judgement: Judgement) -> bool:
    match judgement:
        case Absent() | Identical():
            return False
        case Missing():
            return True
        case Divergent(off_baseline=off_baseline):
            return off_baseline
        case _:
            assert_never(judgement)


def _rival(judgement: Judgement) -> bool:
    match judgement:
        case Absent() | Missing() | Identical():
            return False
        case Divergent(rival=rival):
            return rival
        case _:
            assert_never(judgement)


def _labels(cells: Sequence[Cell]) -> tuple[str, ...]:
    """Unique cell labels.

    >>> _labels([Cell("f", "m3", "O0"), Cell("g", "m3", "O0"), Cell("f", "m3", "O0")])
    ('f@m3/O0', 'g@m3/O0')
    """
    return tuple(dict.fromkeys(f"{fn}@{cpu}/{opt}" for fn, cpu, opt in cells))


def _diff_block(cell: Cell, x: str, y: str, cx: str, cy: str) -> str:
    d = diff_lines(cx, cy, x, y)
    return (
        f"<details><summary>{cell.fn} @ {cell.cpu}/{cell.opt} — {x} vs {y} "
        f"(Δinsn {instr_count(cx) - instr_count(cy):+d})</summary>\n\n```diff\n{d}\n```\n</details>"
        if d
        else ""
    )


def _judge(
    cell: Cell,
    variants: Sequence[str],
    baseline: str,
    candidates: Sequence[str],
    canon: Mapping[Key, str],
    sizes: Mapping[Key, int],
) -> Judgement:
    """Judge one cell.

    >>> v = ["cerive", "handwritten"]
    >>> _judge(Cell("f", "m3", "O0"), v, "handwritten", ["cerive"], {}, {})
    Absent()
    >>> _judge(
    ...     Cell("f", "m3", "O0"),
    ...     v,
    ...     "handwritten",
    ...     ["cerive"],
    ...     {Key("cerive", "m3", "O0", "f"): "nop"},
    ...     {},
    ... )
    Missing()
    """
    bodies = {
        v: canon[Key(v, cell.cpu, cell.opt, cell.fn)]
        for v in variants
        if Key(v, cell.cpu, cell.opt, cell.fn) in canon
    }
    if not bodies:
        return Absent()
    if any(Key(v, cell.cpu, cell.opt, cell.fn) not in canon for v in variants):
        return Missing()
    if len(set(bodies.values())) == 1:
        return Identical()
    rival = len({bodies[v] for v in candidates}) > 1
    candidate_size = sizes.get(Key(candidates[0], cell.cpu, cell.opt, cell.fn))
    baseline_size = sizes.get(Key(baseline, cell.cpu, cell.opt, cell.fn))
    x, y = (
        (candidates[0], candidates[1])
        if rival and len(candidates) >= 2
        else (candidates[0], baseline)
    )
    return Divergent(
        mark="⚠"
        if rival
        else (
            f"{candidate_size - baseline_size:+d}"
            if candidate_size is not None and baseline_size is not None
            else "≠"
        ),
        rival=rival,
        off_baseline=any(bodies[v] != bodies[baseline] for v in candidates),
        diff=_diff_block(cell, x, y, bodies[x], bodies[y]),
    )


def _overview(
    cpus: Sequence[str],
    opts: Sequence[str],
    present: set[tuple[str, str]],
    divergent: Sequence[Cell],
    gaps: Sequence[Gap],
) -> list[str]:
    def cell(cpu: str, opt: str) -> str:
        if any((g.cpu, g.opt) == (cpu, opt) for g in gaps):
            return "❌ incomplete"
        if (cpu, opt) not in present:
            return "-"
        count = len({fn for fn, c, o in divergent if (c, o) == (cpu, opt)})
        return f"❌ {count}" if count else "✅"

    return [
        "functions whose asm differs, per core × optimization level:",
        "",
        "| cpu | " + " | ".join(opts) + " |",
        "|---|" + "---|" * len(opts),
        *(f"| {cpu} | " + " | ".join(cell(cpu, o) for o in opts) + " |" for cpu in cpus),
        "",
    ]


def _section(
    cpu: str,
    variants: Sequence[str],
    opts: Sequence[str],
    fns: Sequence[str],
    baseline: str,
    judgements: Mapping[Cell, Judgement],
    totals: Mapping[Stem, int],
    diverged: bool,
) -> list[str]:
    return [
        f"<details{' open' if diverged else ''}><summary><b>{cpu}</b></summary>",
        "",
        "`text` bytes, whole TU:",
        "",
        "| impl | " + " | ".join(opts) + " |",
        "|---|" + "---|" * len(opts),
        *(
            f"| {v} | " + " | ".join(str(totals.get(Stem(v, cpu, o), "-")) for o in opts) + " |"
            for v in variants
        ),
        "",
        f"per function: `=` identical asm across impls · `+N` Δbytes vs {baseline} · `⚠` candidates disagree · `∅` missing from some impl",
        "",
        "| fn | " + " | ".join(opts) + " |",
        "|---|" + "---|" * len(opts),
        *(
            f"| {fn} | " + " | ".join(_mark(judgements[Cell(fn, cpu, o)]) for o in opts) + " |"
            for fn in fns
            if any(not isinstance(judgements[Cell(fn, cpu, o)], Absent) for o in opts)
        ),
        "",
        "</details>",
        "",
    ]


def render_report(
    variants: Sequence[str],
    cpus: Sequence[str],
    opts: Sequence[str],
    canon: Mapping[Key, str],
    sizes: Mapping[Key, int],
    totals: Mapping[Stem, int],
    evidence_gaps: Sequence[Gap] = (),
) -> Report:
    """Render the evidence report."""
    baseline = "handwritten" if "handwritten" in variants else (variants[-1] if variants else "")
    candidates = [v for v in variants if v != baseline]
    fns = sorted(
        {k.fn for k in canon if not _is_helper(k.fn)},
        key=lambda s: (not s.startswith("study_"), s),
    )
    judgements = {
        cell: _judge(cell, variants, baseline, candidates, canon, sizes)
        for cell in (Cell(fn, cpu, opt) for cpu in cpus for fn in fns for opt in opts)
    }
    strat_breaks = [cell for cell, j in judgements.items() if _rival(j)]
    base_mismatches = [cell for cell, j in judgements.items() if _off_baseline(j)]
    diffs = [j.diff for j in judgements.values() if isinstance(j, Divergent) and j.diff]
    incomplete = (
        *(() if candidates else ("no candidate impl to compare",)),
        *(() if fns else ("no functions compared",)),
        *(gap.reason for gap in evidence_gaps),
    )

    def verdict(breaks: Sequence[Cell], marker: str) -> str:
        if breaks:
            return f"{marker} differ at " + ", ".join(_labels(breaks))
        return "❌ evidence incomplete" if incomplete else "✅ identical everywhere"

    cand_label = " ≡ ".join(candidates) if candidates else "(none)"
    head = [
        "# Evidence — codegen comparison",
        "",
        *(
            [f"**{cand_label}:** " + verdict(strat_breaks, "❌"), ""]
            if len(candidates) >= 2
            else []
        ),
        f"**{cand_label} ≡ {baseline}:** " + verdict(base_mismatches, "❌"),
        "",
        *(["**evidence incomplete:** " + "; ".join(incomplete), ""] if incomplete else []),
        *_overview(
            cpus,
            opts,
            {(k.cpu, k.opt) for k in canon if not _is_helper(k.fn)},
            [*strat_breaks, *base_mismatches],
            evidence_gaps,
        ),
    ]
    body = [
        *(
            line
            for cpu in cpus
            for line in _section(
                cpu,
                variants,
                opts,
                fns,
                baseline,
                judgements,
                totals,
                any(c.cpu == cpu for c in (*strat_breaks, *base_mismatches)),
            )
        ),
        *(["## diffs", "", *diffs] if diffs else []),
    ]
    return Report(
        "\n".join(head + body), (*_labels([*strat_breaks, *base_mismatches]), *incomplete)
    )
