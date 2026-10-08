from cstructs.report import Key, SizeInfo, parse_size, render_report


def test_parse_size() -> None:
    assert parse_size("text data bss\n 88 0 4 92 5c x.o") == SizeInfo(88, 0, 4)


def test_parse_size_rejects_garbage() -> None:
    assert parse_size("") is None


def test_render_all_identical() -> None:
    variants = ["for", "hybrid", "handwritten"]
    canon = {Key(v, "m3", "O2", "study_f"): "mov\tr0" for v in variants}
    out = render_report(variants, ["m3"], ["O2"], canon, {}, {})
    assert out.failures == ()
    assert "✅ identical everywhere" in out.markdown
    assert "| m3 | ✅ |" in out.markdown
    assert "| study_f | = |" in out.markdown
    assert "## diffs" not in out.markdown


def test_render_flags_strategy_divergence() -> None:
    variants = ["for", "hybrid", "handwritten"]
    canon = {
        Key("for", "m3", "O2", "study_f"): "mov\tr0",
        Key("hybrid", "m3", "O2", "study_f"): "mov\tr1",
        Key("handwritten", "m3", "O2", "study_f"): "mov\tr0",
    }
    sizes = {Key("for", "m3", "O2", "study_f"): 4, Key("handwritten", "m3", "O2", "study_f"): 4}
    out = render_report(variants, ["m3"], ["O2"], canon, sizes, {})
    assert out.failures == ("study_f@m3/O2",)
    assert "❌ differ at study_f@m3/O2" in out.markdown
    assert "## diffs" in out.markdown
    assert "⚠" in out.markdown


def test_render_keeps_each_core_of_a_baseline_mismatch() -> None:
    canon = {
        Key(v, cpu, "O0", "f"): body
        for cpu in ("m0plus", "m3")
        for v, body in (("cerive", "mov\tr0\nnop"), ("handwritten", "mov\tr0"))
    }
    out = render_report(["cerive", "handwritten"], ["m0plus", "m3"], ["O0"], canon, {}, {})
    assert out.failures == ("f@m0plus/O0", "f@m3/O0")
    assert "❌ differ at f@m0plus/O0, f@m3/O0" in out.markdown
    assert "| m0plus | ❌ 1 |" in out.markdown
    assert "<details open><summary><b>m3</b></summary>" in out.markdown


def test_render_delta_insn_is_candidate_minus_baseline() -> None:
    canon = {
        Key("cerive", "m3", "O0", "f"): "mov\tr0\nnop",
        Key("handwritten", "m3", "O0", "f"): "mov\tr0",
    }
    sizes = {Key("cerive", "m3", "O0", "f"): 6, Key("handwritten", "m3", "O0", "f"): 4}
    out = render_report(["cerive", "handwritten"], ["m3"], ["O0"], canon, sizes, {})
    assert "| f | +2 |" in out.markdown
    assert "(Δinsn +1)" in out.markdown


def test_render_marks_unbuilt_cells() -> None:
    canon = {Key(v, "m3", "O2", "f"): "mov\tr0" for v in ("cerive", "handwritten")}
    out = render_report(["cerive", "handwritten"], ["m3"], ["O0", "O2"], canon, {}, {})
    assert "| m3 | - | ✅ |" in out.markdown


def test_render_flags_a_function_missing_from_one_impl() -> None:
    canon = {Key("cerive", "m3", "O0", "f"): "mov\tr0"}
    out = render_report(["cerive", "handwritten"], ["m3"], ["O0"], canon, {}, {})
    assert out.failures == ("f@m3/O0",)
    assert "| f | ∅ |" in out.markdown


def test_render_reports_an_empty_comparison() -> None:
    out = render_report(["cerive", "handwritten"], ["m3"], ["O0"], {}, {}, {})
    assert out.failures == ("no functions compared",)
    assert "**cerive ≡ handwritten:** ❌ evidence incomplete" in out.markdown
    assert "**evidence incomplete:** no functions compared" in out.markdown


def test_render_never_certifies_rival_strategies_over_an_empty_matrix() -> None:
    out = render_report(["for", "hybrid", "handwritten"], ["m3"], ["O0"], {}, {}, {})
    assert "**for ≡ hybrid:** ❌ evidence incomplete" in out.markdown
    assert "✅" not in out.markdown.split("functions whose asm differs")[0]


def test_render_needs_a_candidate_to_compare() -> None:
    canon = {Key("cerive", "m3", "O0", "f"): "nop"}
    out = render_report(["cerive"], ["m3"], ["O0"], canon, {}, {})
    assert out.failures == ("no candidate impl to compare",)
    assert "❌ evidence incomplete" in out.markdown


def test_render_publishes_unusable_artifacts() -> None:
    canon = {Key(v, "m3", "O0", "f"): "nop" for v in ("cerive", "handwritten")}
    out = render_report(
        ["cerive", "handwritten"],
        ["m3"],
        ["O0"],
        canon,
        {},
        {},
        ["unusable artifact handwritten.m3.O0.size"],
    )
    assert out.failures == ("unusable artifact handwritten.m3.O0.size",)
    assert "**evidence incomplete:** unusable artifact handwritten.m3.O0.size" in out.markdown
