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


def test_report_fails_and_publishes_a_missing_artifact(tmp_path: Path) -> None:
    for v in VARIANTS:
        write_cell(tmp_path, v, "nop")
    (tmp_path / "handwritten.m3.O0.size").unlink()
    summary = tmp_path / "summary.md"
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"], summary) == 1
    assert "unusable artifact handwritten.m3.O0.size" in (tmp_path / "report.md").read_text()
    assert "❌ evidence incomplete" in summary.read_text()


def test_report_fails_on_artifacts_that_parse_to_nothing(tmp_path: Path) -> None:
    for v in VARIANTS:
        write_cell(tmp_path, v, "nop")
    (tmp_path / "cerive.m3.O0.sym").write_text("")
    (tmp_path / "handwritten.m3.O0.size").write_text("garbage\n")
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"]) == 1
    published = (tmp_path / "report.md").read_text()
    assert "unusable artifact cerive.m3.O0.sym" in published
    assert "unusable artifact handwritten.m3.O0.size" in published


def test_report_fails_when_nothing_was_compared(tmp_path: Path) -> None:
    assert report(tmp_path, VARIANTS, [], ["O0"]) == 1


def test_unwritable_summary_does_not_decide_the_verdict(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    for v in VARIANTS:
        write_cell(tmp_path, v, "nop")
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"], tmp_path) == 0
    assert "job summary not written" in capsys.readouterr().err


def test_report_fails_on_a_function_with_no_instructions(tmp_path: Path) -> None:
    for v in VARIANTS:
        write_cell(tmp_path, v, ".cfi_startproc")
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"]) == 1
    assert "unusable artifact cerive.m3.O0.s" in (tmp_path / "report.md").read_text()


def test_report_fails_when_asm_and_symbols_disagree(tmp_path: Path) -> None:
    for v in VARIANTS:
        write_cell(tmp_path, v, "nop")
    (tmp_path / "cerive.m3.O0.sym").write_text("00000000 00000002 T f\n00000002 00000002 T g\n")
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"]) == 1
    assert (
        "cerive.m3.O0.s and cerive.m3.O0.sym define different functions"
        in (tmp_path / "report.md").read_text()
    )


def test_report_publishes_an_undecodable_artifact(tmp_path: Path) -> None:
    for v in VARIANTS:
        write_cell(tmp_path, v, "nop")
    (tmp_path / "handwritten.m3.O0.s").write_bytes(b"\xff\xfe")
    assert report(tmp_path, VARIANTS, ["m3"], ["O0"]) == 1
    assert "unusable artifact handwritten.m3.O0.s" in (tmp_path / "report.md").read_text()


def test_report_publishes_into_a_missing_matrix_dir(tmp_path: Path) -> None:
    matrix = tmp_path / "never-built"
    assert report(matrix, VARIANTS, ["m3"], ["O0"]) == 1
    assert "❌ evidence incomplete" in (matrix / "report.md").read_text()
