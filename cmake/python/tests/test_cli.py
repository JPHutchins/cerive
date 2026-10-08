from typing import TYPE_CHECKING

from cstructs.cli import report

if TYPE_CHECKING:
    from pathlib import Path

    import pytest

VARIANTS = ["cerive", "handwritten"]


def write_cell(matrix: Path, variant: str, body: str) -> None:
    (matrix / f"{variant}.m3.O0.s").write_text(
        f"\t.type f, %function\nf:\n\t{body}\n\t.size f, .-f\n"
    )
    (matrix / f"{variant}.m3.O0.sym").write_text("00000000 00000002 T f\n")
    (matrix / f"{variant}.m3.O0.size").write_text(
        "text data bss dec hex filename\n 2 0 0 2 2 x.o\n"
    )


def test_report_passes_and_publishes_identical_matrix(tmp_path: Path) -> None:
    for v in VARIANTS:
        write_cell(tmp_path, v, "nop")
    summary = tmp_path / "summary.md"
    summary.write_text("earlier step\n")
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"], summary) == 0
    assert summary.read_text().startswith("earlier step\n# Evidence")
    assert "✅ identical everywhere" in (tmp_path / "report.md").read_text()


def test_report_fails_on_divergence(tmp_path: Path) -> None:
    write_cell(tmp_path, "cerive", "nop")
    write_cell(tmp_path, "handwritten", "bx\tlr")
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"]) == 1


def test_report_fails_on_missing_artifact(tmp_path: Path) -> None:
    for v in VARIANTS:
        write_cell(tmp_path, v, "nop")
    (tmp_path / "handwritten.m3.O0.size").unlink()
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"]) == 1


def test_report_fails_when_nothing_was_compared(tmp_path: Path) -> None:
    assert report(tmp_path, VARIANTS, [], ["O0"]) == 1


def test_unwritable_summary_does_not_decide_the_verdict(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    for v in VARIANTS:
        write_cell(tmp_path, v, "nop")
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"], tmp_path) == 0
    assert "job summary not written" in capsys.readouterr().err
