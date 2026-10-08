"""README example extraction."""

import re

_C_BLOCK = re.compile(r"^```c\n(.*?)^```$", re.MULTILINE | re.DOTALL)


def c_examples(markdown: str) -> str:
    r"""The fenced C blocks of a markdown document, as one translation unit.

    >>> c_examples("x\n```c\nint a;\n```\ny\n```sh\nls\n```\n```c\nint b;\n```\n")
    'int a;\n\nint b;\n'
    >>> c_examples("no code")
    ''
    """
    return "\n".join(_C_BLOCK.findall(markdown))
