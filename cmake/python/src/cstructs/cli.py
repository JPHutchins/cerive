"""Command-line entry points."""

import subprocess
import sys
import tarfile
from collections.abc import Mapping
from pathlib import Path
from typing import Annotated, NamedTuple

from cyclopts import App, Parameter

from cstructs.asm import canonical, parse_syms, split_functions
from cstructs.expand import strip_system_headers
from cstructs.readme import c_examples
from cstructs.release import assets, gate, problem
from cstructs.report import Gap, Key, Stem, parse_size, render_report

app = App(name="cstructs", help="cerive build tooling.")

type Tokens = Annotated[list[str], Parameter(consume_multiple=True)]


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


class Evidence(NamedTuple):
    """One artifact stem's evidence."""

    stem: Stem
    bodies: Mapping[str, str]
    syms: Mapping[str, int]
    text: int | None
    gaps: tuple[Gap, ...]


def _evidence(
    stem: Stem, s_text: str | None, sym_text: str | None, size_text: str | None
) -> Evidence:
    r"""Parse one artifact stem.

    >>> [g.reason for g in _evidence(Stem("cerive", "m3", "O0"), None, "", "junk").gaps]
    ['unusable artifact cerive.m3.O0.s', 'unusable artifact cerive.m3.O0.sym', 'unusable artifact cerive.m3.O0.size']
    >>> _evidence(
    ...     Stem("cerive", "m3", "O0"),
    ...     "\t.type f, %function\nf:\n\tnop\n\t.size f, .-f\n",
    ...     "00000000 00000002 T g\n",
    ...     " 2 0 0 2 2 x.o\n",
    ... ).gaps[0].reason
    'cerive.m3.O0.s and cerive.m3.O0.sym define different functions'
    """
    bodies = {fn: canonical(raw) for fn, raw in split_functions(s_text or "").items()}
    syms = parse_syms(sym_text or "")
    size = parse_size(size_text or "")
    s_usable = bool(bodies) and all(bodies.values())
    return Evidence(
        stem=stem,
        bodies=bodies if s_usable else {},
        syms=syms,
        text=size.text if size is not None else None,
        gaps=(
            *(
                Gap(stem.cpu, stem.opt, f"unusable artifact {stem}.{ext}")
                for ext, usable in (
                    ("s", s_usable),
                    ("sym", bool(syms)),
                    ("size", size is not None),
                )
                if not usable
            ),
            *(
                (Gap(stem.cpu, stem.opt, f"{stem}.s and {stem}.sym define different functions"),)
                if bodies and syms and set(bodies) != set(syms)
                else ()
            ),
        ),
    )


@app.command
def report(
    matrix_dir: Path,
    variants: Tokens,
    cpus: Tokens,
    opts: Tokens,
    summary: Annotated[Path | None, Parameter(env_var="GITHUB_STEP_SUMMARY")] = None,
) -> int:
    """Render, publish and gate on the evidence report."""
    evidence = [
        _evidence(
            stem,
            _read(matrix_dir / f"{stem}.s"),
            _read(matrix_dir / f"{stem}.sym"),
            _read(matrix_dir / f"{stem}.size"),
        )
        for stem in (Stem(v, c, o) for v in variants for c in cpus for o in opts)
    ]
    rendered = render_report(
        variants,
        cpus,
        opts,
        {Key(*e.stem, fn): body for e in evidence for fn, body in e.bodies.items()},
        {Key(*e.stem, fn): size for e in evidence for fn, size in e.syms.items()},
        {e.stem: e.text for e in evidence if e.text is not None},
        [gap for e in evidence for gap in e.gaps],
    )
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
    out.write_text(result.stdout, encoding="utf-8")
    return 0


type Tag = Annotated[str, Parameter(env_var="GITHUB_REF_NAME")]


@app.command
def version_gate(header: Path, tag: Tag) -> int:
    """Gate a release tag on the version in cerive.h."""
    text = _read(header)
    reason = problem(gate(text, tag)) if text is not None else f"cannot read {header}"
    if reason is not None:
        print(f"::error::{reason}")
        return 1
    print(f"{header} declares {tag}")
    return 0


@app.command
def release_assets(matrix_dir: Path, out: Path, tag: Tag) -> int:
    """Write the evidence assets of a release to OUT."""
    evidence = _read(matrix_dir / "report.md")
    if evidence is None:
        print(f"::error::no evidence report in {matrix_dir}")
        return 1
    names = assets(tag)
    out.mkdir(parents=True, exist_ok=True)
    (out / names.report).write_text(evidence, encoding="utf-8")
    with tarfile.open(out / names.matrix, "w:gz") as archive:
        archive.add(matrix_dir, arcname=names.matrix.removesuffix(".tar.gz"))
    print(f"wrote {out / names.report} and {out / names.matrix}")
    return 0
