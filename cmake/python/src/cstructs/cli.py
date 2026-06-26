"""Command-line entry points; logic lives in the pure functions they call."""

import subprocess
import sys
from pathlib import Path
from typing import Annotated

from cyclopts import App, Parameter

from cstructs.expand import strip_system_headers
from cstructs.report import SizeInfo, parse_size, render_report

app = App(name="cstructs", help="String transforms backing the c-structs CMake build.")

Tokens = Annotated[list[str], Parameter(consume_multiple=True)]


@app.command
def expand(file: Path) -> None:
    """Strip system-header noise from a preprocessed file, in place."""
    file.write_text(strip_system_headers(file.read_text()))


@app.command
def report(matrix_dir: Path, variants: Tokens, cpus: Tokens, opts: Tokens) -> None:
    """Aggregate per-cell *.size files into a markdown comparison table."""
    sizes: dict[tuple[str, str, str], SizeInfo | None] = {
        (variant, cpu, opt): _read_size(matrix_dir / f"{variant}.{cpu}.{opt}.size")
        for variant in variants
        for cpu in cpus
        for opt in opts
    }
    table = render_report(variants, cpus, opts, sizes)
    (matrix_dir / "report.md").write_text(table + "\n")
    print(f"\n{table}\n")
    print(f"artifacts: {matrix_dir}/  (*.i expansion, *.s asm, *.lst disasm, *.size)")


@app.command
def capture(out: Path, *command: str) -> int:
    """Run COMMAND, writing its stdout to --out (replaces shell redirects)."""
    result = subprocess.run(command, capture_output=True, text=True)
    sys.stderr.write(result.stderr)
    if result.returncode == 0:
        out.write_text(result.stdout)
    return result.returncode


def _read_size(path: Path) -> SizeInfo | None:
    return parse_size(path.read_text()) if path.exists() else None
