from cstructs.report import SizeInfo, parse_size, render_report


def test_parse_size() -> None:
    assert parse_size("text data bss\n 88 0 4 92 5c x.o") == SizeInfo(88, 0, 4)


def test_parse_size_rejects_garbage() -> None:
    assert parse_size("") is None


def test_render_all_identical() -> None:
    variants = ["for", "hybrid", "handwritten"]
    canon = {(v, "m3", "O2", "study_f"): "mov\tr0" for v in variants}
    out = render_report(variants, ["m3"], ["O2"], canon, {}, {})
    assert out.divergences == ()
    assert "✅ identical everywhere" in out.markdown
    assert "| m3 | ✅ |" in out.markdown
    assert "| study_f | = |" in out.markdown
    assert "## diffs" not in out.markdown


def test_render_flags_strategy_divergence() -> None:
    variants = ["for", "hybrid", "handwritten"]
    canon = {
        ("for", "m3", "O2", "study_f"): "mov\tr0",
        ("hybrid", "m3", "O2", "study_f"): "mov\tr1",
        ("handwritten", "m3", "O2", "study_f"): "mov\tr0",
    }
    sizes = {("for", "m3", "O2", "study_f"): 4, ("handwritten", "m3", "O2", "study_f"): 4}
    out = render_report(variants, ["m3"], ["O2"], canon, sizes, {})
    assert out.divergences == ("study_f@m3/O2",)
    assert "❌ differ at study_f@m3/O2" in out.markdown
    assert "## diffs" in out.markdown
    assert "⚠" in out.markdown


def test_render_keeps_each_core_of_a_baseline_mismatch() -> None:
    canon = {
        (v, cpu, "O0", "f"): body
        for cpu in ("m0plus", "m3")
        for v, body in (("cerive", "mov\tr0\nnop"), ("handwritten", "mov\tr0"))
    }
    out = render_report(["cerive", "handwritten"], ["m0plus", "m3"], ["O0"], canon, {}, {})
    assert out.divergences == ("f@m0plus/O0", "f@m3/O0")
    assert "⚠️ differ at f@m0plus/O0, f@m3/O0" in out.markdown
    assert "| m0plus | ⚠️ 1 |" in out.markdown
    assert "<details open><summary><b>m3</b></summary>" in out.markdown


def test_render_delta_insn_is_candidate_minus_baseline() -> None:
    canon = {
        ("cerive", "m3", "O0", "f"): "mov\tr0\nnop",
        ("handwritten", "m3", "O0", "f"): "mov\tr0",
    }
    sizes = {("cerive", "m3", "O0", "f"): 6, ("handwritten", "m3", "O0", "f"): 4}
    out = render_report(["cerive", "handwritten"], ["m3"], ["O0"], canon, sizes, {})
    assert "| f | +2 |" in out.markdown
    assert "(Δinsn +1)" in out.markdown


def test_render_marks_unbuilt_cells() -> None:
    canon = {(v, "m3", "O2", "f"): "mov\tr0" for v in ("cerive", "handwritten")}
    out = render_report(["cerive", "handwritten"], ["m3"], ["O0", "O2"], canon, {}, {})
    assert "| m3 | - | ✅ |" in out.markdown


def test_render_flags_a_function_missing_from_one_impl() -> None:
    canon = {("cerive", "m3", "O0", "f"): "mov\tr0"}
    out = render_report(["cerive", "handwritten"], ["m3"], ["O0"], canon, {}, {})
    assert out.divergences == ("f@m3/O0",)
    assert "| f | ∅ |" in out.markdown


def test_render_reports_an_empty_comparison() -> None:
    out = render_report(["cerive", "handwritten"], ["m3"], ["O0"], {}, {}, {})
    assert out.functions == 0
    assert "❌ no functions compared" in out.markdown
