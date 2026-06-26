# c-structs

A test bench for emulating Rust's `#[derive(...)]` in embedded C23 with X-macros,
and for gathering **codegen evidence** about whether that abstraction is free.

A "deriveable" type is described once by a field list; generator macros expand it
into the struct plus its methods (`Debug`, `new`, `Default`, `PartialEq`).
Tagged unions are derived the same way from a variant list, where each variant
token names its union member and member struct, plus a `_tag`-suffixed enum
discriminant (three C namespaces) so construction and matching are pure token
reuse while the field structs stay `typedef`'d. The
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
| `*.s` | compiler assembly (the diff source) |
| `*.lst` | object disassembly (`objdump -d -S`) — for browsing, not diffing |
| `*.size` | whole-TU `text` / `data` / `bss` |
| `*.sym` | per-function sizes (`nm --print-size`) |

and `build/matrix/report.md`: a top verdict (`for ≡ hybrid`? `≡ handwritten`?), a
per-function **equivalence grid** (`=` identical asm · `+N` Δbytes vs baseline ·
`⚠` candidates disagree), and, for any divergent cell, a **normalized unified
asm diff** (helper names and local labels neutralized so only real codegen
differences show). The comparison logic lives in the typed, tested
[cstructs.asm](cmake/python/src/cstructs/asm.py) / `report` modules.

The seeded `Point`/`Line`/`Frame` already show the result: both derive styles
(`for`, `hybrid`) produce **byte-identical** code to the hand-written baseline at
every optimization level, including the recursive nested `Frame` (`diff
build/matrix/for.*.O2.s build/matrix/hybrid.*.O2.s` is empty, likewise vs
`handwritten`). `Debug` is the one capability with real cost — it pulls in
`snprintf` everywhere — kept for contrast. Reaching parity required the generated
constructors to take `const` by-value parameters; without it a nested struct
field forces a defensive copy — caught by the matrix at `-O1+`, not the `-O0`
tests.

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

- **Another implementation to compare:** add `variants/<name>/shapes.h` exposing
  the same type API, then append `<name>` to `-DVARIANTS=...`. It automatically
  gains a QEMU test and matrix rows.
- **Another deriveable type:** give it a `T_FIELDS` list and the `DERIVE_*` calls
  in each impl's [variants/](variants/) header, then exercise it from
  [study/study.c](study/study.c) and [tests/test_shapes.c](tests/test_shapes.c).
- **Another derive (or requirement):** add a generator to both
  [derive/derive_for.h](derive/derive_for.h) and
  [derive/derive_hybrid.h](derive/derive_hybrid.h). Keeping the two 1:1 *is* the
  experiment — pile on requirements until one approach loses viability.

## Tagged unions

A union is a variant list whose members are its member struct types:

```c
#define Shape_VARIANTS Point, Line, Frame      /* derive_for; hybrid: (X) X(Point)... */
DERIVE_UNION(Shape);
DERIVE_UNION_DEBUG(Shape)
DERIVE_UNION_PARTIAL_EQ(Shape)
#define Shape_new(...) UNION_NEW(Shape, __VA_ARGS__)
```

This generates `enum Shape_tag` (discriminants `Point_tag`, …), the anonymous
`union` + `tag`, and `Shape_debug` / `Shape_eq` that `switch` on the tag and
recurse into each variant's derive. The helpers in [derive/union.h](derive/union.h)
exploit member ≡ struct name, with a `_tag`-suffixed discriminant pasted by the
macros (so you always pass the bare token):

```c
Shape s = Shape_new(Point, .x = 1, .y = 2);   /* construct */
if (UNION_IS(s, Point)) { /* ... */ }          /* predicate  */
MATCH (s) {                                    /* type-safe match */
    CASE (Point, p) { use(p->x); }   /* p is `Point const *`, bound from s */
    CASE (Frame, f) { use(f->id); }  /* wrong-field access won't compile */
}
```

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
derive/                   two feature-equal derive frameworks (same type API):
  derive_for.h              FOR_EACH / __VA_OPT__ unroll (comma-tuple fields)
  derive_hybrid.h           classic operator-threaded + DROP1 (no field-count cap)
  union.h                   generic tagged-union helpers (UNION_NEW/IS, MATCH/CASE)
variants/<impl>/shapes.h  for | hybrid | handwritten — Point/Line/Frame, compared
study/study.c             stable entry points so the codegen is emitted
tests/test_shapes.c       correctness + sizing/truncation, run under QEMU per impl
.clangd .vscode/ .envrc   IDE: clangd over build/compile_commands.json
```
