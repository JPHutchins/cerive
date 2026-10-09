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
cfg_arm = Task("cmake --preset arm", mutates=True)
build = Task("cmake --build build", mutates=True)
ctest = Task(
    "ctest --preset arm",
    agent_format=("--output-junit {report}", "junit"),
    mutates=True,
)
build_evidence = Task("cmake --build --preset evidence", mutates=True)
cfg_analyze = Task("cmake --preset analyze", mutates=True)
build_analyze = Task("cmake --build --preset analyze", mutates=True)
analyze = Sequential(cfg_analyze, build_analyze, when=C_BUILD_INPUTS)
c = Sequential(cfg_arm, build, ctest, when=C_BUILD_INPUTS)
evidence = Sequential(cfg_arm, build, ctest, build_evidence, when=C_BUILD_INPUTS)
cfg_host = Task(
    "nix shell --inputs-from . nixpkgs#{PKG} --command"
    " cmake --preset host -B build-host/{PKG} -DCMAKE_C_COMPILER={CC}",
    mutates=True,
)
build_host = Task("cmake --build build-host/{PKG}", mutates=True)
ctest_host = Task(
    "ctest --test-dir build-host/{PKG} --output-on-failure",
    agent_format=("--output-junit {report}", "junit"),
    mutates=True,
)
host = Parallel(
    Sequential(cfg_host, build_host, ctest_host),
    variants=HOST_COMPILERS,
    when=C_BUILD_INPUTS,
)
nix = Task("nix flake check --print-build-logs", when=("flake.nix", "flake.lock"))
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
