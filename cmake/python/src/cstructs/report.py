"""Render a glanceable codegen comparison: equivalence grid + true asm diffs."""

import re
from typing import Mapping, NamedTuple, Sequence

from cstructs.asm import diff_lines, instr_count

Key = tuple[str, str, str, str]  # impl, cpu, opt, function


class SizeInfo(NamedTuple):
    text: int
    data: int
    bss: int


def parse_size(output: str) -> SizeInfo | None:
    """Parse the data row of `size` (Berkeley). First three ints are text/data/bss.

    >>> parse_size("  text  data  bss\\n  88  0  4  92  5c  x.o")
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


def render_report(
    variants: Sequence[str],
    cpus: Sequence[str],
    opts: Sequence[str],
    canon: Mapping[Key, str],
    sizes: Mapping[Key, int],
    totals: Mapping[tuple[str, str, str], int],
) -> str:
    """Build report.md. `canon`/`sizes` are keyed (impl, cpu, opt, fn); `totals`
    is whole-TU text bytes keyed (impl, cpu, opt). handwritten is the baseline;
    the remaining variants are the candidate strategies compared against it."""
    baseline = "handwritten" if "handwritten" in variants else (variants[-1] if variants else "")
    candidates = [v for v in variants if v != baseline]
    fns = sorted(
        {k[3] for k in canon if not _is_helper(k[3])},
        key=lambda s: (not s.startswith("study_"), s),
    )

    diffs: list[str] = []
    strat_breaks: list[str] = []
    base_breaks: list[str] = []
    body: list[str] = []

    for cpu in cpus:
        body += [f"## {cpu}", "", "`text` bytes, whole TU:", "", "| impl | " + " | ".join(opts) + " |", "|---|" + "---|" * len(opts)]
        for v in variants:
            body.append(f"| {v} | " + " | ".join(str(totals.get((v, cpu, o), "-")) for o in opts) + " |")
        body += [
            "",
            f"per function: `=` identical asm across impls · `+N` Δbytes vs {baseline} · `⚠` candidates disagree",
            "",
            "| fn | " + " | ".join(opts) + " |",
            "|---|" + "---|" * len(opts),
        ]
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
                (strat_breaks if strat_diff else base_breaks).append(f"{fn}@{opt}")
                sz_a = sizes.get((candidates[0], cpu, opt, fn)) if candidates else None
                sz_b = sizes.get((baseline, cpu, opt, fn))
                delta = f"{sz_a - sz_b:+d}" if sz_a is not None and sz_b is not None else "≠"
                cells.append(("⚠" if strat_diff else "") + delta)
                x, y = (cand_here[0], cand_here[1]) if strat_diff and len(cand_here) >= 2 else (candidates[0], baseline)
                cx, cy = canon.get((x, cpu, opt, fn)), canon.get((y, cpu, opt, fn))
                if cx is not None and cy is not None:
                    d = diff_lines(cx, cy, x, y)
                    if d:
                        diffs.append(
                            f"<details><summary>{fn} @ {cpu}/{opt} — {x} vs {y} "
                            f"(Δinsn {instr_count(cy) - instr_count(cx):+d})</summary>\n\n```diff\n{d}\n```\n</details>"
                        )
            if present:
                body.append(f"| {fn} | " + " | ".join(cells) + " |")
        body.append("")

    cand_label = " ≡ ".join(candidates) if candidates else "(none)"
    head = ["# Evidence — codegen comparison", ""]
    if len(candidates) >= 2:  # only meaningful with rival strategies to agree/disagree
        head += [
            f"**{cand_label}:** "
            + ("✅ identical everywhere" if not strat_breaks else "❌ differ at " + ", ".join(dict.fromkeys(strat_breaks))),
            "",
        ]
    head += [
        f"**{cand_label} ≡ {baseline}:** "
        + ("✅ identical everywhere" if not base_breaks else "⚠️ differ at " + ", ".join(dict.fromkeys(base_breaks))),
        "",
    ]
    if diffs:
        body += ["## diffs", ""] + diffs
    return "\n".join(head + body)
