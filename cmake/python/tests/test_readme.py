from cstructs.readme import c_examples


def test_c_examples_keeps_only_c_fences() -> None:
    markdown = "```sh\ncamas\n```\n\n```c\nint main(void) { return 0; }\n```\n"
    assert c_examples(markdown) == "int main(void) { return 0; }\n"


def test_c_examples_ignores_indented_and_unterminated_fences() -> None:
    assert c_examples("    ```c\n    int a;\n    ```\n```c\nint b;\n") == ""
