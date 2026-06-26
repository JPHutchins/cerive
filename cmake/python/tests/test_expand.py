from cstructs.expand import strip_system_headers


def test_drops_system_keeps_project() -> None:
    src = (
        '# 1 "study.c"\n'
        "typedef struct Point { int x; } Point;\n"
        '# 1 "/nix/store/x/stdio.h" 1 3 4\n'
        "int system_decl;\n"
        '# 2 "study.c" 2\n'
        "Point p;\n"
    )
    assert strip_system_headers(src) == "typedef struct Point { int x; } Point;\nPoint p;\n"


def test_drops_builtins() -> None:
    assert strip_system_headers('# 1 "<built-in>"\nx\n') == ""


def test_empty() -> None:
    assert strip_system_headers("") == ""
