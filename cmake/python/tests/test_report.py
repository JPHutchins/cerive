from cstructs.report import SizeInfo, parse_size, render_report


def test_parse_size() -> None:
    assert parse_size("   text   data    bss\n  88     0     4    92    5c  x.o\n") == SizeInfo(88, 0, 4)


def test_parse_size_rejects_garbage() -> None:
    assert parse_size("") is None


def test_render_marks_missing_cells() -> None:
    out = render_report(["d", "h"], ["m3"], ["O2"], {("d", "m3", "O2"): SizeInfo(98, 0, 0)})
    assert "| d | 98 |" in out
    assert "| h | - |" in out
