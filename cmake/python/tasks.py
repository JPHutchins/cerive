"""camas task definitions."""

from camas import Claude, Config, Parallel, Sequential, Task, by_suffix

python_sources = by_suffix((".py",))

lint = Task(
    "uv run ruff check {paths}",
    paths=python_sources,
    agent_format=("--output-format sarif", "sarif"),
)
format_check = Task("uv run ruff format --check {paths}", paths=python_sources)
mypy = Task("uv run mypy {paths}", paths=python_sources)
pyright = Task("uv run pyright {paths}", paths=python_sources)
test = Task("uv run pytest -v", agent_format=("--junitxml {report}", "junit"))
lint_fix = Task("uv run ruff check --fix {paths}", paths=python_sources, mutates=True)
format_fix = Task("uv run ruff format {paths}", paths=python_sources, mutates=True)

check = Parallel(lint, format_check, mypy, pyright, test)
fix = Sequential(lint_fix, format_fix)
all = Sequential(fix, check)

_ = Config(default_task=all, github_task=check, agent=Claude(fix=fix, check=check))
