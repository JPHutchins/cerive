"""Command-line entry points."""

import subprocess
import sys
from pathlib import Path
from typing import Annotated

from cyclopts import App, Parameter

from cstructs.asm import canonical, parse_syms, split_functions
from cstructs.expand import strip_system_headers
from cstructs.readme import c_examples
from cstructs.report import Key, parse_size, render_report

app = App(name="cstructs", help="cerive build tooling.")

Tokens = Annotated[list[str], Parameter(consume_multiple=True)]


@app.command
def expand(file: Path) -> None:
    """Strip system headers from a preprocessed file."""
    try:
        file.write_text(strip_system_headers(file.read_text()))
    except FileNotFoundError:
        print(f"error: {file} not found", file=sys.stderr)
        sys.exit(1)
    except PermissionError:
        print(f"error: permission denied: {file}", file=sys.stderr)
        sys.exit(1)


def _read(path: Path) -> str | None:
    try:
        return path.read_text(encoding="utf-8")
    except OSError, UnicodeDecodeError:
        return None


@app.command
def report(
    matrix_dir: Path,
    variants: Tokens,
    cpus: Tokens,
    opts: Tokens,
    summary: Annotated[Path | None, Parameter(env_var="GITHUB_STEP_SUMMARY")] = None,
) -> int:
    """Render, publish and gate on the evidence report."""
    evidence_gaps: list[str] = []
    canon: dict[Key, str] = {}
    sizes: dict[Key, int] = {}
    totals: dict[tuple[str, str, str], int] = {}
    for v in variants:
        for c in cpus:
            for o in opts:
                stem = f"{v}.{c}.{o}"
                s_text, sym_text, size_text = (
                    _read(matrix_dir / f"{stem}.{ext}") for ext in ("s", "sym", "size")
                )
                bodies = {
                    fn: canonical(raw)
                    for fn, raw in (split_functions(s_text) if s_text is not None else {}).items()
                }
                syms = parse_syms(sym_text) if sym_text is not None else {}
                info = parse_size(size_text) if size_text is not None else None
                evidence_gaps += [
                    f"unusable artifact {name}"
                    for name, usable in (
                        (f"{stem}.s", bool(bodies) and all(bodies.values())),
                        (f"{stem}.sym", bool(syms)),
                        (f"{stem}.size", info is not None),
                    )
                    if not usable
                ]
                if bodies and syms and set(bodies) != set(syms):
                    evidence_gaps.append(f"{stem}.s and {stem}.sym define different functions")
                canon.update({Key(v, c, o, fn): body for fn, body in bodies.items()})
                sizes.update({Key(v, c, o, fn): size for fn, size in syms.items()})
                if info is not None:
                    totals[(v, c, o)] = info.text
    rendered = render_report(variants, cpus, opts, canon, sizes, totals, evidence_gaps)
    matrix_dir.mkdir(parents=True, exist_ok=True)
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
def readme_c(readme: Path, out: Path) -> None:
    """Write the C examples of README to --out."""
    out.write_text(c_examples(readme.read_text(encoding="utf-8")), encoding="utf-8")


@app.command
def capture(out: Path, *command: str) -> int:
    """Run COMMAND, writing its stdout to --out."""
    result = subprocess.run(command, capture_output=True, text=True, check=False)
    sys.stderr.write(result.stderr)
    if result.returncode != 0:
        print(f"error: command failed with exit code {result.returncode}", file=sys.stderr)
        return result.returncode
    out.write_text(result.stdout)
    return 0
