"""Parse `size` output and render the variant x opt code-size comparison."""

import re
from typing import Mapping, NamedTuple, Sequence


class SizeInfo(NamedTuple):
    text: int
    data: int
    bss: int


def parse_size(output: str) -> SizeInfo | None:
    """Parse the data row of `arm-none-eabi-size` (Berkeley format).

    The header row carries no digits, so the first three integers are
    `text`, `data`, `bss`.

    >>> parse_size("   text   data    bss\\n     88      0      4     92    5c\\n")
    SizeInfo(text=88, data=0, bss=4)
    >>> parse_size("nonsense") is None
    True
    """
    numbers = re.findall(r"\d+", output)
    if len(numbers) < 3:
        return None
    return SizeInfo(int(numbers[0]), int(numbers[1]), int(numbers[2]))


def render_report(
    variants: Sequence[str],
    cpus: Sequence[str],
    opts: Sequence[str],
    sizes: Mapping[tuple[str, str, str], SizeInfo | None],
) -> str:
    """Render a markdown table of `text` bytes, one section per cpu.

    >>> render_report(["d"], ["m3"], ["O2"], {("d", "m3", "O2"): SizeInfo(98, 0, 0)})
    '# Evidence matrix - code size in bytes (text segment)\\n\\n## m3\\n\\n| variant | O2 |\\n|---|---|\\n| d | 98 |\\n'
    """

    def text_of(variant: str, cpu: str, opt: str) -> str:
        info = sizes.get((variant, cpu, opt))
        return str(info.text) if info is not None else "-"

    header = ["# Evidence matrix - code size in bytes (text segment)", ""]
    sections = [
        line
        for cpu in cpus
        for line in (
            f"## {cpu}",
            "",
            "| variant | " + " | ".join(opts) + " |",
            "|---|" + "---|" * len(opts),
            *(
                "| " + variant + " | "
                + " | ".join(text_of(variant, cpu, opt) for opt in opts)
                + " |"
                for variant in variants
            ),
            "",
        )
    ]
    return "\n".join(header + sections)
