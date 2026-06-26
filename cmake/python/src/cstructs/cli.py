"""Command-line entry points; logic lives in the pure functions they call."""

import subprocess
import sys
from pathlib import Path
from typing import Annotated

from cyclopts import App, Parameter

from cstructs.asm import canonical, parse_syms, split_functions
from cstructs.expand import strip_system_headers
from cstructs.report import Key, parse_size, render_report

app = App(name="cstructs", help="String transforms backing the c-structs CMake build.")

Tokens = Annotated[list[str], Parameter(consume_multiple=True)]


@app.command
def expand(file: Path) -> None:
    """Strip system-header noise from a preprocessed file, in place."""
    file.write_text(strip_system_headers(file.read_text()))


@app.command
def report(matrix_dir: Path, variants: Tokens, cpus: Tokens, opts: Tokens) -> None:
    """Render the equivalence grid + asm diffs from the matrix artifacts."""
    canon: dict[Key, str] = {}
    sizes: dict[Key, int] = {}
    totals: dict[tuple[str, str, str], int] = {}
    for v in variants:
        for c in cpus:
            for o in opts:
                s_path = matrix_dir / f"{v}.{c}.{o}.s"
                sym_path = matrix_dir / f"{v}.{c}.{o}.sym"
                size_path = matrix_dir / f"{v}.{c}.{o}.size"
                if s_path.exists():
                    for fn, raw in split_functions(s_path.read_text()).items():
                        canon[(v, c, o, fn)] = canonical(raw)
                if sym_path.exists():
                    for fn, size in parse_syms(sym_path.read_text()).items():
                        sizes[(v, c, o, fn)] = size
                if size_path.exists():
                    info = parse_size(size_path.read_text())
                    if info is not None:
                        totals[(v, c, o)] = info.text
    table = render_report(variants, cpus, opts, canon, sizes, totals)
    (matrix_dir / "report.md").write_text(table + "\n")
    print(f"\n{table}\n")
    print(f"full report: {matrix_dir}/report.md")


@app.command
def capture(out: Path, *command: str) -> int:
    """Run COMMAND, writing its stdout to --out (replaces shell redirects)."""
    result = subprocess.run(command, capture_output=True, text=True)
    sys.stderr.write(result.stderr)
    if result.returncode == 0:
        out.write_text(result.stdout)
    return result.returncode
