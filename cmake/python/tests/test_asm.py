from cstructs.asm import canonical, diff_lines, instr_count, parse_syms, split_functions


def test_canonical_neutralizes_helpers_and_labels() -> None:
    a = canonical("\tbl\tcerive_buf_remaining\n.L9:\n\tbx\tlr")
    b = canonical("\tbl\thw_rem\n.L3:\n\tbx\tlr")
    assert a == b


def test_canonical_keeps_real_differences() -> None:
    assert canonical("\tmovs\tr0, #0") != canonical("\tmovs\tr0, #1")


def test_split_functions() -> None:
    asm = "\t.type g, %function\ng:\n\tnop\n\t.size g, .-g\n"
    assert set(split_functions(asm)) == {"g"}


def test_parse_syms_takes_sized_only() -> None:
    assert parse_syms("00000000 00000018 T study_eq\n0000abcd t local") == {"study_eq": 24}


def test_diff_empty_when_equal() -> None:
    assert diff_lines("a\nb", "a\nb", "x", "y") == ""
    assert diff_lines("a", "b", "x", "y") != ""


def test_instr_count_ignores_labels_and_data() -> None:
    assert instr_count("mov\tr0, r1\n.L0:\n.word\t0\nbx\tlr") == 2
