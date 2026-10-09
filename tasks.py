"""camas task definitions."""

from pathlib import Path

from camas import Claude, Config, Parallel, Project, Sequential, Task, by_glob

ROOT = Path(__file__).parent
C_TREE = ("bsp", "include", "src", "study", "tests", "variants")
C_GLOBS = tuple(f"{tree}/**/*.{ext}" for tree in C_TREE for ext in ("c", "h"))
C_BUILD_INPUTS = (
    *C_TREE,
    "cmake",
    "CMakeLists.txt",
    "CMakePresets.json",
    "flake.nix",
    "flake.lock",
    "tasks.py",
)
HOST_COMPILERS = (
    {"PKG": "gcc13", "CC": "gcc"},
    {"PKG": "clang_18", "CC": "clang"},
)

c_sources = by_glob(
    C_GLOBS,
    default=tuple(p.relative_to(ROOT).as_posix() for g in C_GLOBS for p in sorted(ROOT.glob(g))),
)

cfmt = Task("jphfmt --check {paths}", paths=c_sources)
cfg_arm = Task("cmake --preset arm")
build = Task("cmake --build build")
ctest = Task("ctest --preset arm", agent_format=("--output-junit {report}", "junit"))
build_evidence = Task("cmake --build --preset evidence")
cfg_analyze = Task("cmake --preset analyze")
build_analyze = Task("cmake --build --preset analyze")
analyze = Sequential(cfg_analyze, build_analyze, when=C_BUILD_INPUTS)
c = Sequential(cfg_arm, build, ctest, when=C_BUILD_INPUTS)
evidence = Sequential(cfg_arm, build, ctest, build_evidence, when=C_BUILD_INPUTS)
cfg_host = Task(
    "nix shell --inputs-from . nixpkgs#{PKG} --command"
    " cmake --preset host -B build-host/{PKG} -DCMAKE_C_COMPILER={CC}"
)
build_host = Task("cmake --build build-host/{PKG}")
ctest_host = Task(
    "ctest --test-dir build-host/{PKG} --output-on-failure",
    agent_format=("--output-junit {report}", "junit"),
)
host = Parallel(
    Sequential(cfg_host, build_host, ctest_host),
    variants=HOST_COMPILERS,
    when=C_BUILD_INPUTS,
)
nix = Task(
    "nix flake check --print-build-logs",
    when=(*C_BUILD_INPUTS, "Kconfig", "LICENSE", "zephyr"),
)
check = Parallel(cfmt, evidence, analyze, host)
fix = Task("jphfmt -i {paths}", paths=c_sources, mutates=True)
default = Sequential(fix, check)

cmake_python = Project("cmake/python")

_ = Config(
    default_task=Parallel(default, cmake_python, name="dev"),
    github_task=Parallel(check, nix, cmake_python, name="ci"),
    agent=Claude(
        fix=Parallel(fix, cmake_python, name="agent_fix"),
        check=Parallel(check, cmake_python, name="agent_check"),
    ),
)
