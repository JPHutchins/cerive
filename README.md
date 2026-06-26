# c-structs

A test bench for emulating Rust's `#[derive(...)]` in embedded C23 with X-macros,
and for gathering **codegen evidence** about whether that abstraction is free.

A "deriveable" type is described once by a field list; generator macros expand it
into the struct plus its methods (`Debug`, `new`, `Default`, `PartialEq`). The
harness then (a) proves each derive is **correct** by running it on ARM under
QEMU, and (b) emits a **comparison matrix** — preprocessed expansion, assembly,
disassembly and segment sizes — for the derived code versus a hand-written
equivalent, across optimization levels and Cortex-M cores.

## Toolchain

Everything is pinned by the flake; you need only Nix with flakes enabled.

- `arm-none-eabi-gcc` 15.2 (full C23), newlib-nano, semihosting specs
- `qemu-system-arm` (bare-metal test runner, `lm3s6965evb` / Cortex-M3)
- CMake + Ninja, `astyle` (pretty-prints the expansion)
- `uv` + Python 3.14 — the `cstructs` package ([cmake/python/](cmake/python/))
  holds the string transforms CMake delegates to, so CMake stays declarative

```sh
nix develop
```

## Use

```sh
cmake --preset arm                 # configure (Ninja + arm-none-eabi toolchain)
cmake --build build                # build the QEMU test binaries
ctest --preset arm                 # run the derives on ARM, under QEMU
cmake --build --preset evidence    # generate the matrix + print the size report
```

Tests pass/fail by their **process exit code**: each test's `main` returns its
failure count, the bare-metal harness forwards it through the semihosting
`SYS_EXIT`, and QEMU exits with it. Correctness and evidence are intentionally
split: tests *run* on one proven core; the matrix only *compiles*, so its core
axis is free to range over parts that no QEMU machine models.

### Evidence artifacts

`cmake --build --preset evidence` writes, per `variant × cpu × opt`, into
[build/matrix/](build/matrix/):

| file | contents |
|---|---|
| `*.i` | macro expansion ([study/study.c](study/study.c)), system headers stripped |
| `*.s` | compiler assembly |
| `*.lst` | object disassembly (`objdump -d -S`) |
| `*.size` | `text` / `data` / `bss` |

and a `text`-size comparison table to stdout and `build/matrix/report.md`.

The seeded `Point` already shows the result: all three derive styles produce
byte-for-byte identical code to the hand-written version from `-O1` up
(`diff build/matrix/derive.*.O2.s build/matrix/handwritten.*.O2.s` is empty). At
`-O0` only `xmacro` differs (+2 bytes) — its constructor workaround passes a
sacrificial argument the others avoid. `Debug` pulls in `snprintf` in every
variant; it is the one derive whose cost is real, kept for contrast.

### Sweeping the axes

Cache variables, set at configure time:

```sh
cmake --preset arm \
  -DMATRIX_CPUS="cortex-m0plus;cortex-m3;cortex-m4;cortex-m7" \
  -DMATRIX_OPTS="O0;O2;Os;Oz"
cmake --build --preset evidence
```

`ARM_CPU` / `QEMU_MACHINE` select the test core; the default pair
(`cortex-m3` / `lm3s6965evb`) is the well-trodden semihosting target — change
both together.

## Editing (clangd)

`cmake --preset arm` exports `build/compile_commands.json` (covering the BSP,
tests and `study.c` per variant). [.clangd](.clangd) points clangd at it and
[.vscode/settings.json](.vscode/settings.json) allows the `arm-none-eabi-gcc`
driver so newlib includes resolve. Run `direnv allow` so the editor inherits the
flake's toolchain, or launch the editor from inside `nix develop`.

## Working on the tooling

CMake performs only build-graph wiring; every string transform is a pure,
statically-typed function in [cmake/python/](cmake/python/), exercised independently:

```sh
cd cmake/python
uv run mypy       # strict
uv run pytest     # units + doctests
```

## Adding to the bench

- **Another implementation to compare:** add `variants/<name>/point.h` exposing
  the same `Point` API, then append `<name>` to `-DVARIANTS=...`. It
  automatically gains a QEMU test and matrix rows.
- **Another deriveable type:** give it a `T_FIELDS` macro and the `DERIVE_*`
  calls (see [variants/derive/point.h](variants/derive/point.h)), then exercise
  it from [study/study.c](study/study.c) and [tests/test_point.c](tests/test_point.c).
- **Another derive:** add a generator to [derive/derive.h](derive/derive.h). The
  `FOR_EACH` / `FOR_EACH_C` mappers (comma-free and comma-separated) handle the
  field iteration.

## Layout

```
flake.nix                 dev shell (toolchain, qemu, cmake/ninja, astyle)
CMakeLists.txt            tests (CTest+QEMU) and evidence targets
CMakePresets.json         `arm` configure / `evidence` build / `arm` test presets
cmake/
  arm-none-eabi.cmake     cross toolchain file
  matrix.cmake            add_matrix(): per-cell build-graph wiring only
cmake/python/             uv project `cstructs`: the typed string transforms
  src/cstructs/expand.py    strip system headers from preprocessed output
  src/cstructs/report.py    parse *.size, render the comparison table
  src/cstructs/cli.py       thin cyclopts shims that CMake invokes
  tests/                    pytest units (+ doctests in the modules)
bsp/                      bare-metal harness: vector table, semihosting, linker
derive/                   three derive frameworks, one Point API:
  derive.h                  FOR_EACH / __VA_OPT__ unroll (comma-tuple fields)
  derive_xmacro.h           classic operator-threaded X-macro (+ ctor shim)
  derive_hybrid.h           classic + DROP1 leading-comma strip for `new`
variants/<name>/point.h   derive | xmacro | hybrid | handwritten — under comparison
study/study.c             stable entry points so the codegen is emitted
study/study.c             stable entry points so the codegen is emitted
tests/test_point.c        correctness checks, run under QEMU per variant
.clangd .vscode/ .envrc   IDE: clangd over build/compile_commands.json
```
