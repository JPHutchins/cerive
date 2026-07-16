"""camas task definitions."""

from pathlib import Path

from camas import Claude, Config, Parallel, Sequential, Task, Project

ROOT = Path(__file__).parent
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

def c_build_touched(changed: tuple[str, ...]) -> bool:
    """`when=` for the cmake lanes: a C source, the cmake wiring, or the toolchain changed."""
    return any(
        p.startswith(tuple(f"{d}/" for d in C_TREE))
        or p.endswith(".cmake")
        or p in ("CMakeLists.txt", "CMakePresets.json", "flake.nix", "flake.lock")
        for p in changed
    )

cfmt = Task("jphfmt --check {paths}", paths=c_sources)
cfg_arm = Task("cmake --preset arm")
build = Task("cmake --build build")
ctest = Task("ctest --preset arm")
build_evidence = Task("cmake --build --preset evidence")
clang_analyze = Task("cmake --build build --target clang-analyze", when=c_build_touched)
cfg_analyze = Task("cmake --preset analyze")
build_analyze = Task("cmake --build --preset analyze")
analyze = Sequential(cfg_analyze, build_analyze, when=c_build_touched)
c = Sequential(cfg_arm, build, ctest, when=c_build_touched)
evidence = Sequential(cfg_arm, build, ctest, build_evidence, when=c_build_touched)
nix = Task("nix flake check --print-build-logs", when=("flake.nix", "flake.lock"))
check = Parallel(cfmt, evidence, c, analyze)
fix = Task("jphfmt -i {paths}", paths=c_sources, mutates=True)
default = Sequential(fix, check)

cmake_python = Project("cmake/python")

_ = Config(
    default_task=Parallel(default, cmake_python),
    github_task=Parallel(check, nix, cmake_python),
    agent=Claude(fix=Parallel(fix, cmake_python), check=Parallel(check, cmake_python))
)
