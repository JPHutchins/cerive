import tarfile
from pathlib import Path

import pytest

from cstructs.cli import release_assets, version_gate
from cstructs.release import header_version

CERIVE_H = Path(__file__).parents[3] / "include" / "cerive" / "cerive.h"

HEADER = "#define CERIVE_VERSION_MAJOR 0\n#define CERIVE_VERSION_MINOR 1\n#define CERIVE_VERSION_PATCH 0\n"


def test_cerive_h_declares_a_complete_version() -> None:
    assert header_version(CERIVE_H.read_text(encoding="utf-8")) is not None


def test_version_gate_passes_the_declared_tag(tmp_path: Path) -> None:
    header = tmp_path / "cerive.h"
    header.write_text(HEADER)
    assert version_gate(header, "v0.1.0") == 0


def test_version_gate_annotates_a_mismatched_tag(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    header = tmp_path / "cerive.h"
    header.write_text(HEADER)
    assert version_gate(header, "v0.2.0") == 1
    assert capsys.readouterr().out == "::error::cerive.h is v0.1.0, the tag is v0.2.0\n"


def test_version_gate_fails_on_an_unreadable_header(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    assert version_gate(tmp_path / "missing.h", "v0.1.0") == 1
    assert capsys.readouterr().out.startswith("::error::cannot read ")


def test_release_assets_publish_the_report_and_the_matrix(tmp_path: Path) -> None:
    matrix = tmp_path / "matrix"
    matrix.mkdir()
    (matrix / "report.md").write_text("# Evidence\n")
    (matrix / "cerive.m3.O0.s").write_text("nop\n")
    out = tmp_path / "assets"
    assert release_assets(matrix, out, "v0.1.0") == 0
    assert (out / "cerive-v0.1.0-evidence.md").read_text() == "# Evidence\n"
    with tarfile.open(out / "cerive-v0.1.0-evidence-matrix.tar.gz") as archive:
        assert set(archive.getnames()) == {
            "cerive-v0.1.0-evidence-matrix",
            "cerive-v0.1.0-evidence-matrix/report.md",
            "cerive-v0.1.0-evidence-matrix/cerive.m3.O0.s",
        }


def test_release_assets_fail_without_an_evidence_report(tmp_path: Path) -> None:
    assert release_assets(tmp_path, tmp_path / "assets", "v0.1.0") == 1
    assert not (tmp_path / "assets").exists()
