"""camas task definitions."""

from camas import Claude, Config, Parallel, Sequential, Task

lint = Task("uv run ruff check {paths}", paths=".")
format_check = Task("uv run ruff format --check {paths}", paths=".")
mypy = Task("uv run mypy {paths}", paths=".")
pyright = Task("uv run pyright {paths}", paths=".")
test = Task("uv run pytest -v")
lint_fix = Task("uv run ruff check --fix {paths}", paths=".", mutates=True)
format_fix = Task("uv run ruff format {paths}", paths=".", mutates=True)

check = Parallel(lint, format_check, mypy, pyright, test)
fix = Sequential(lint_fix, format_fix)
all = Sequential(fix, check)

_ = Config(default_task=all, github_task=check, agent=Claude(fix=fix, check=check))
