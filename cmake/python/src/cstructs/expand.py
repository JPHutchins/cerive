"""Reduce `gcc -E` output to project code, dropping system-header noise."""

import re

_MARKER = re.compile(r'^# \d+ "([^"]*)"(.*)$')


def _is_system(path: str, flags: str) -> bool:
    return path.startswith("<") or "3" in flags.split()


def strip_system_headers(preprocessed: str) -> str:
    """Keep only lines whose enclosing `# line "file"` marker is project code.

    System headers carry the `3` flag and built-ins start with `<`, so only
    code pulled in via `-I` (the derive expansion) survives.

    >>> strip_system_headers('# 1 "a.c"\\nkeep me\\n# 1 "h.h" 3 4\\ndrop me\\n')
    'keep me\\n'
    """
    kept: list[str] = []
    keep = False
    for line in preprocessed.splitlines():
        marker = _MARKER.match(line)
        if marker is not None:
            keep = not _is_system(marker.group(1), marker.group(2))
        elif keep:
            kept.append(line)
    return _tidy("\n".join(kept))


def _tidy(text: str) -> str:
    """Drop leading/trailing blank lines and collapse interior blank runs.

    >>> _tidy('\\n\\n\\na\\n\\n\\n\\nb')
    'a\\n\\nb\\n'
    """
    body = re.sub(r"\n{3,}", "\n\n", text.strip("\n"))
    return body + "\n" if body else ""
