"""Preprocessed-output filtering."""

import re
from itertools import accumulate

_MARKER = re.compile(r'^# \d+ "([^"]*)"(.*)$')


def _is_system(path: str, flags: str) -> bool:
    return path.startswith("<") or "3" in flags.split()


def _keeping_after(keeping: bool, line: str) -> bool:
    marker = _MARKER.match(line)
    return keeping if marker is None else not _is_system(marker.group(1), marker.group(2))


def _project_lines(lines: list[str]) -> str:
    return "\n".join(
        line
        for line, keeping in zip(
            lines, accumulate(lines, _keeping_after, initial=False), strict=False
        )
        if keeping and _MARKER.match(line) is None
    )


def strip_system_headers(preprocessed: str) -> str:
    r"""Strip system headers from preprocessed output.

    >>> strip_system_headers('# 1 "a.c"\nkeep me\n# 1 "h.h" 3 4\ndrop me\n')
    'keep me\n'
    >>> strip_system_headers('# 1 "a.c"\na\n# 2 "<built-in>"\nb\n# 3 "a.c" 2\nc\n')
    'a\nc\n'
    """
    return _tidy(_project_lines(preprocessed.splitlines()))


def _tidy(text: str) -> str:
    r"""Normalize blank lines.

    >>> _tidy("\n\n\na\n\n\n\nb")
    'a\n\nb\n'
    """
    body = re.sub(r"\n{3,}", "\n\n", text.strip("\n"))
    return body + "\n" if body else ""
