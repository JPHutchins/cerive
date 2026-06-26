from cstructs.report import SizeInfo, parse_size, render_report


def test_parse_size() -> None:
    assert parse_size("text data bss\n 88 0 4 92 5c x.o") == SizeInfo(88, 0, 4)


def test_parse_size_rejects_garbage() -> None:
    assert parse_size("") is None


def test_render_all_identical() -> None:
    variants = ["for", "hybrid", "handwritten"]
    canon = {(v, "m3", "O2", "study_f"): "mov\tr0" for v in variants}
    out = render_report(variants, ["m3"], ["O2"], canon, {}, {})
    assert "✅ identical everywhere" in out
    assert "| study_f | = |" in out
    assert "## diffs" not in out


def test_render_flags_strategy_divergence() -> None:
    variants = ["for", "hybrid", "handwritten"]
    canon = {
        ("for", "m3", "O2", "study_f"): "mov\tr0",
        ("hybrid", "m3", "O2", "study_f"): "mov\tr1",
        ("handwritten", "m3", "O2", "study_f"): "mov\tr0",
    }
    sizes = {("for", "m3", "O2", "study_f"): 4, ("handwritten", "m3", "O2", "study_f"): 4}
    out = render_report(variants, ["m3"], ["O2"], canon, sizes, {})
    assert "❌ differ at study_f@O2" in out
    assert "## diffs" in out
    assert "⚠" in out
