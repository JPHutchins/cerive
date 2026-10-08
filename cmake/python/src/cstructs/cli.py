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
    try:
        file.write_text(strip_system_headers(file.read_text()))
    except FileNotFoundError:
        print(f"error: {file} not found", file=sys.stderr)
        sys.exit(1)
    except PermissionError:
        print(f"error: permission denied: {file}", file=sys.stderr)
        sys.exit(1)


@app.command
def report(
    matrix_dir: Path,
    variants: Tokens,
    cpus: Tokens,
    opts: Tokens,
    summary: Annotated[Path | None, Parameter(env_var="GITHUB_STEP_SUMMARY")] = None,
) -> int:
    """Render the equivalence grid + asm diffs from the matrix artifacts, appending them to
    --summary (the GitHub job summary under Actions); fails when the candidates diverge or
    the evidence is incomplete.
    """
    unusable: list[str] = []
    canon: dict[Key, str] = {}
    sizes: dict[Key, int] = {}
    totals: dict[tuple[str, str, str], int] = {}
    for v in variants:
        for c in cpus:
            for o in opts:
                s_path = matrix_dir / f"{v}.{c}.{o}.s"
                sym_path = matrix_dir / f"{v}.{c}.{o}.sym"
                size_path = matrix_dir / f"{v}.{c}.{o}.size"
                functions = split_functions(s_path.read_text()) if s_path.exists() else {}
                syms = parse_syms(sym_path.read_text()) if sym_path.exists() else {}
                info = parse_size(size_path.read_text()) if size_path.exists() else None
                unusable += [
                    path.name
                    for path, usable in (
                        (s_path, bool(functions)),
                        (sym_path, bool(syms)),
                        (size_path, info is not None),
                    )
                    if not usable
                ]
                canon.update({(v, c, o, fn): canonical(raw) for fn, raw in functions.items()})
                sizes.update({(v, c, o, fn): size for fn, size in syms.items()})
                if info is not None:
                    totals[(v, c, o)] = info.text
    rendered = render_report(variants, cpus, opts, canon, sizes, totals, unusable)
    (matrix_dir / "report.md").write_text(rendered.markdown + "\n", encoding="utf-8")
    if summary is not None:
        try:
            with summary.open("a", encoding="utf-8") as f:
                f.write(rendered.markdown + "\n")
        except OSError as e:
            print(f"warning: job summary not written: {e}", file=sys.stderr)
    print(f"\n{rendered.markdown}\n")
    print(f"full report: {matrix_dir}/report.md")
    return 1 if rendered.failures else 0


@app.command
def capture(out: Path, *command: str) -> int:
    """Run COMMAND, writing its stdout to --out (replaces shell redirects)."""
    result = subprocess.run(command, capture_output=True, text=True, check=False)
    sys.stderr.write(result.stderr)
    if result.returncode != 0:
        print(f"error: command failed with exit code {result.returncode}", file=sys.stderr)
        return result.returncode
    out.write_text(result.stdout)
    return 0
