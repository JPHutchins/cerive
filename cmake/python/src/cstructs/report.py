"""Render a glanceable codegen comparison: equivalence grid + true asm diffs."""

import re
from typing import TYPE_CHECKING, NamedTuple

from cstructs.asm import diff_lines, instr_count

if TYPE_CHECKING:
    from collections.abc import Mapping, Sequence

Key = tuple[str, str, str, str]  # impl, cpu, opt, function
Cell = tuple[str, str, str]  # function, cpu, opt


class SizeInfo(NamedTuple):
    """Berkeley `size` segment byte counts for one translation unit."""

    text: int
    data: int
    bss: int


def parse_size(output: str) -> SizeInfo | None:
    r"""Parse the data row of `size` (Berkeley). First three ints are text/data/bss.

    >>> parse_size("  text  data  bss\n  88  0  4  92  5c  x.o")
    SizeInfo(text=88, data=0, bss=4)
    >>> parse_size("nonsense") is None
    True
    """
    numbers = re.findall(r"\d+", output)
    if len(numbers) < 3:
        return None
    return SizeInfo(int(numbers[0]), int(numbers[1]), int(numbers[2]))


def _is_helper(fn: str) -> bool:
    return fn.startswith(("cerive_", "hw_"))


class Report(NamedTuple):
    """report.md, and every `fn@cpu/opt` cell that breaks the equivalence verdict."""

    markdown: str
    divergences: tuple[str, ...]


def _labels(cells: Sequence[Cell]) -> tuple[str, ...]:
    """`fn@cpu/opt`, deduplicated in first-seen order.

    >>> _labels([("f", "m3", "O0"), ("g", "m3", "O0"), ("f", "m3", "O0")])
    ('f@m3/O0', 'g@m3/O0')
    """
    return tuple(dict.fromkeys(f"{fn}@{cpu}/{opt}" for fn, cpu, opt in cells))


def _overview(
    cpus: Sequence[str],
    opts: Sequence[str],
    present: set[tuple[str, str]],
    divergent: Sequence[Cell],
) -> list[str]:
    def cell(cpu: str, opt: str) -> str:
        if (cpu, opt) not in present:
            return "-"
        count = len({fn for fn, c, o in divergent if (c, o) == (cpu, opt)})
        return f"⚠️ {count}" if count else "✅"

    return [
        "functions whose asm differs, per core × optimization level:",
        "",
        "| cpu | " + " | ".join(opts) + " |",
        "|---|" + "---|" * len(opts),
        *(f"| {cpu} | " + " | ".join(cell(cpu, o) for o in opts) + " |" for cpu in cpus),
        "",
    ]


def render_report(
    variants: Sequence[str],
    cpus: Sequence[str],
    opts: Sequence[str],
    canon: Mapping[Key, str],
    sizes: Mapping[Key, int],
    totals: Mapping[tuple[str, str, str], int],
) -> Report:
    """Build report.md. `canon`/`sizes` are keyed (impl, cpu, opt, fn); `totals`
    is whole-TU text bytes keyed (impl, cpu, opt). handwritten is the baseline;
    the remaining variants are the candidate strategies compared against it.
    """
    baseline = "handwritten" if "handwritten" in variants else (variants[-1] if variants else "")
    candidates = [v for v in variants if v != baseline]
    fns = sorted(
        {k[3] for k in canon if not _is_helper(k[3])},
        key=lambda s: (not s.startswith("study_"), s),
    )

    diffs: list[str] = []
    strat_breaks: list[Cell] = []
    base_mismatches: list[Cell] = []
    body: list[str] = []

    for cpu in cpus:
        grid: list[str] = []
        for fn in fns:
            cells: list[str] = []
            present = False
            for opt in opts:
                got = {v: canon.get((v, cpu, opt, fn)) for v in variants}
                have = [v for v in variants if got[v] is not None]
                if not have:
                    cells.append("")
                    continue
                present = True
                if len({got[v] for v in have}) == 1:
                    cells.append("=")
                    continue
                cand_here = [v for v in candidates if got[v] is not None]
                strat_diff = len({got[v] for v in cand_here}) > 1
                if strat_diff:
                    strat_breaks.append((fn, cpu, opt))
                    cells.append("⚠")
                else:
                    sz_a = sizes.get((candidates[0], cpu, opt, fn)) if candidates else None
                    sz_b = sizes.get((baseline, cpu, opt, fn))
                    delta = f"{sz_a - sz_b:+d}" if sz_a is not None and sz_b is not None else "≠"
                    cells.append(delta)
                baseline_got = got.get(baseline)
                if baseline_got is not None and any(
                    got[v] is not None and got[v] != baseline_got for v in cand_here
                ):
                    base_mismatches.append((fn, cpu, opt))
                x, y = (
                    (cand_here[0], cand_here[1])
                    if strat_diff and len(cand_here) >= 2
                    else (candidates[0], baseline)
                )
                cx, cy = canon.get((x, cpu, opt, fn)), canon.get((y, cpu, opt, fn))
                if cx is not None and cy is not None:
                    d = diff_lines(cx, cy, x, y)
                    if d:
                        diffs.append(
                            f"<details><summary>{fn} @ {cpu}/{opt} — {x} vs {y} "
                            f"(Δinsn {instr_count(cx) - instr_count(cy):+d})</summary>\n\n```diff\n{d}\n```\n</details>"
                        )
            if present:
                grid.append(f"| {fn} | " + " | ".join(cells) + " |")
        diverged_here = any(c == cpu for _, c, _ in (*strat_breaks, *base_mismatches))
        body += [
            f"<details{' open' if diverged_here else ''}><summary><b>{cpu}</b></summary>",
            "",
            "`text` bytes, whole TU:",
            "",
            "| impl | " + " | ".join(opts) + " |",
            "|---|" + "---|" * len(opts),
            *(
                f"| {v} | " + " | ".join(str(totals.get((v, cpu, o), "-")) for o in opts) + " |"
                for v in variants
            ),
            "",
            f"per function: `=` identical asm across impls · `+N` Δbytes vs {baseline} · `⚠` candidates disagree",
            "",
            "| fn | " + " | ".join(opts) + " |",
            "|---|" + "---|" * len(opts),
            *grid,
            "",
            "</details>",
            "",
        ]

    cand_label = " ≡ ".join(candidates) if candidates else "(none)"
    head = ["# Evidence — codegen comparison", ""]
    if len(candidates) >= 2:  # only meaningful with rival strategies to agree/disagree
        head += [
            f"**{cand_label}:** "
            + (
                "✅ identical everywhere"
                if not strat_breaks
                else "❌ differ at " + ", ".join(_labels(strat_breaks))
            ),
            "",
        ]
    head += [
        f"**{cand_label} ≡ {baseline}:** "
        + (
            "✅ identical everywhere"
            if not base_mismatches
            else "⚠️ differ at " + ", ".join(_labels(base_mismatches))
        ),
        "",
        *_overview(cpus, opts, {(k[1], k[2]) for k in canon}, [*strat_breaks, *base_mismatches]),
    ]
    if diffs:
        body += ["## diffs", "", *diffs]
    return Report("\n".join(head + body), _labels([*strat_breaks, *base_mismatches]))
