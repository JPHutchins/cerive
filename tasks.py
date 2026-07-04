"""camas task definitions — the single source of truth for cerive validation.

Devs (`camas`), CI (`camas check`), and agents (the MCP gate + the PostToolBatch autofix)
all run the same leaves composed here; nothing duplicates the pipeline. Each module-level
binding's variable name is its task name (`camas <name>`); `Parallel` is independent
read-only work (wall-clock max), `Sequential` is real ordering (a build dir mutated in
steps, or a step that consumes a prior's output).

- C correctness on ARM under QEMU (configure, build, ctest) plus clang `--analyze` and a
  separate GCC `-fanalyzer` build;
- the `cstructs` Python tooling — ruff (strict), mypy, pyright, pytest — and jphfmt over the
  C sources;
- `nix flake check` and the evidence matrix, heavy enough to sit in the full run only.

`gate` is the fast per-edit loop the agent drives; `all` is the comprehensive run for devs
and CI. `fix` is the mutating autofix node the PostToolBatch hook applies to changed files.
"""

from pathlib import Path

from camas import Claude, Config, Parallel, Sequential, Task

ROOT = Path(__file__).parent
PY = Path("cmake/python")
C_TREE = ("bsp", "include", "src", "study", "tests", "variants")


def c_sources(changed: tuple[str, ...]) -> tuple[str, ...]:
    """jphfmt scope: the changed C sources/headers, or the whole tracked C tree on a full run."""
    if changed:
        return tuple(p for p in changed if p.endswith((".c", ".h")))
    return tuple(
        p.relative_to(ROOT).as_posix()
        for d in C_TREE
        for p in sorted((ROOT / d).rglob("*"))
        if p.suffix in {".c", ".h"}
    )


def py_sources(changed: tuple[str, ...]) -> tuple[str, ...]:
    """ruff scope: the changed cstructs Python files; empty on a full run so ruff walks its cwd."""
    return tuple(p for p in changed if p.endswith(".py") and p.startswith("cmake/python/"))


# --- Python tooling (cstructs, cmake/python/) ---
ruff = Task("uv run ruff check {paths}", cwd=PY, paths=py_sources)
ruff_format = Task("uv run ruff format --check {paths}", cwd=PY, paths=py_sources)
mypy = Task("uv run mypy", cwd=PY)
pyright = Task("uv run pyright", cwd=PY)
pytest = Task("uv run pytest", cwd=PY)
python = Parallel(ruff, ruff_format, mypy, pyright, pytest)

# --- C formatting (jphfmt) ---
cfmt = Task("jphfmt --check {paths}", paths=c_sources)

# --- C build/test leaves (ARM / QEMU, build dir `build`) ---
cfg_arm = Task("cmake --preset arm")
build = Task("cmake --build build")
ctest = Task("ctest --preset arm")
build_evidence = Task("cmake --build --preset evidence")
# clang static analyzer. Deliberately NOT in `gate`/`all`: clang 21 rejects the C23
# declaration-as-condition `if (T x = expr)` that MATCH/CASE/IF_LET use (HEAD 8aca771) — an
# extension gcc-arm accepts and clang does not — so this target cannot pass with the current
# toolchain. Runnable via `camas clang_analyze`; fold it back into `c`/`evidence` once the
# macros compile under clang (or a clang-compatible spelling replaces the if-decl).
clang_analyze = Task("cmake --build build --target clang-analyze")

# --- C static analysis (GCC -fanalyzer, separate build dir `build-analyze`) ---
cfg_analyze = Task("cmake --preset analyze")
build_analyze = Task("cmake --build --preset analyze")
analyze = Sequential(cfg_analyze, build_analyze)

# --- C lanes (share the `build` dir, so each is a single ordered chain, never parallel) ---
c = Sequential(cfg_arm, build, ctest)
evidence = Sequential(cfg_arm, build, ctest, build_evidence)

# --- Nix ---
# current system only: --all-systems would evaluate the ARM toolchain (gcc-arm-embedded, qemu)
# for platforms where it is unsupported and fail. CI runs this per-runner-OS instead.
nix = Task("nix flake check --print-build-logs")

# --- Gates (composed from the leaves above; no duplicated pipeline) ---
gate = Parallel(python, cfmt, c, analyze)
all = Parallel(python, cfmt, evidence, analyze, nix)

# --- Agent autofix node: mutating, behavior-preserving, scoped to changed files ---
ruff_fix = Task("uv run ruff check --fix {paths}", cwd=PY, paths=py_sources, mutates=True)
ruff_write = Task("uv run ruff format {paths}", cwd=PY, paths=py_sources, mutates=True)
cfmt_fix = Task("jphfmt -i {paths}", paths=c_sources, mutates=True)
fix = Sequential(ruff_fix, ruff_write, cfmt_fix)

_ = Config(default_task=all, github_task=all, agent=Claude(fix=fix, check=gate))
