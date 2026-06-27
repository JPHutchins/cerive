# cerive

`cerive` derives struct methods — `Debug`, `new`, `Default`, `PartialEq`, `Ord`,
`Hash` — and type-safe tagged unions from one field list, in standard embedded
C23 X-macros. It is header-only ([include/cerive/](include/cerive/)), and the
generated code is intended to be **byte-identical to hand-written** at every
optimization level. This repo is both the library and the evidence bench that
proves that claim.

A type is described once by a field list; `CERIVE(...)` expands it into the struct
plus the named methods:

```c
#define Frame_FIELDS(X) \
	X(Line, edge) \
	X(int32_t, id)
CERIVE(Frame, Struct, Debug, Ctor, Default, PartialEq, Ord, Hash)
```

Tagged unions are derived the same way from a variant list, where each variant
token names its union member, its member struct, and (with a `_tag` suffix) its
enum discriminant — three C namespaces — so construction and matching are pure
token reuse while the field structs stay `typedef`'d.

The bench then (a) proves each derive is **correct** by running it on ARM under
QEMU, and (b) emits a **comparison matrix** — preprocessed expansion, assembly,
disassembly and segment sizes — for the derived code versus a hand-written
equivalent, across optimization levels and Cortex-M cores.

That bench settled the design: two X-macro strategies were raced 1:1 — a
`FOR_EACH`/`__VA_OPT__` unroll and classic operator-threading — against the
hand-written baseline. They stayed byte-identical through every capability except
one: the unroll has a hard field-count ceiling (~41), while threading has none.
**Threading won**; it is the sole framework now. The unroll variant lives in git
history.

## Use as a library

Header-only, so just put [include/](include/) on your include path and
`#include <cerive/cerive.h>`. With CMake (`add_subdirectory` or `FetchContent`):

```cmake
add_subdirectory(cerive)
target_link_libraries(app PRIVATE cerive::cerive)   # C23 + the include path
```

As a **Zephyr module**, point `west`/`ZEPHYR_EXTRA_MODULES` at this repo and
`CONFIG_CERIVE=y`; [zephyr/module.yml](zephyr/module.yml) adds the include path
(no object code). The rest of this README is the bench.

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

and `build/matrix/report.md`: a top verdict (`cerive ≡ handwritten`?), a
per-function **equivalence grid** (`=` identical asm · `+N` Δbytes vs baseline ·
`⚠` candidates disagree), and, for any divergent cell, a **normalized unified
asm diff** (helper names and local labels neutralized so only real codegen
differences show). The comparison logic lives in the typed, tested
[cstructs.asm](cmake/python/src/cstructs/asm.py) / `report` modules.

The seeded `Point`/`Line`/`Frame`/`Span`/`Boxed` show the result: the derived code
is **byte-identical** to the hand-written baseline at every optimization level —
flat structs, recursive nesting (`Frame`), pointer fields (`Span`), const members
(`Boxed`), tagged-union dispatch (`Shape`) — across `Debug`/`new`/`Default`/
`PartialEq`/`Ord`/`Hash` (`diff build/matrix/hybrid.*.O2.s build/matrix/handwritten.*.O2.s`
is empty). `Debug` is the one capability with real cost — it pulls in `snprintf`
everywhere — kept for contrast. Reaching parity required the generated
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

## Defining a type

A field list plus one `CERIVE(...)` naming the traits to generate (`Struct` must
lead — it defines the type the rest reference). The field kind is inferred, not
tagged: `(type, name)` is a scalar or `record` (by-value struct), `(type const,
name)` a const record member, `(*, type, name)` a by-address pointer (the `*`
reads literally — `(*, Point *, p)` is a `Point **`):

```c
#define Frame_FIELDS(X) \
    X(Line, edge) \
    X(int32_t, id)
CERIVE(Frame, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

Frame const f = CERIVE_NEW(Frame, .edge = {.a = {1, 2}, .b = {3, 4}}, .id = 7);
```

`Constructor` generates the positional `Frame_new(Line, int32_t)`; `CERIVE_NEW` is
the fluent compound-literal form (designated init, omitted fields zero). Scalar
formats come from the compile-time registry in
[include/cerive/field.h](include/cerive/field.h) — add an entry for a new scalar.

## Adding to the bench

- **Another implementation to compare:** add `variants/<name>/shapes.h` exposing
  the same type API, then append `<name>` to `-DVARIANTS=...`. It automatically
  gains a QEMU test and matrix rows. (`handwritten` is the baseline; the rest are
  candidates compared against it.)
- **Another deriveable type:** give it a `T_FIELDS(X)` list and a `CERIVE(...)`
  in the [variants/cerive/](variants/cerive/) header (hand-mirror it in
  `handwritten`), then exercise it from [study/study.c](study/study.c) and
  [tests/test_shapes.c](tests/test_shapes.c).
- **Another derive:** add a `CERIVE_<Name>` generator to
  [include/cerive/cerive.h](include/cerive/cerive.h) (a per-field handler set for
  the `scalar` / `record` / `const_record` / `pointer` kinds) and list `<Name>` in
  the `CERIVE(...)` call.

## Tagged unions

A union is a variant list whose members are its member struct types:

```c
#define Shape_VARIANTS(X) X(Point) X(Line) X(Frame)
CERIVE_UNION(Shape, Debug, PartialEq)   /* fans out like CERIVE; struct/enum implied */
#define Shape_new(...) CERIVE_UNION_NEW(Shape, __VA_ARGS__)
```

This generates `enum Shape_tag` (discriminants `Point_tag`, …), the anonymous
`union` + `tag`, and `Shape_debug` / `Shape_eq` that `switch` on the tag and
recurse into each variant's derive. The helpers in
[include/cerive/union.h](include/cerive/union.h) exploit member ≡ struct name,
with a `_tag`-suffixed discriminant pasted by the macros (so you always pass the
bare token):

```c
Shape s = Shape_new(Point, .x = 1, .y = 2);     /* construct */
if (CERIVE_IS(s, Point)) { /* ... */ }           /* predicate  */
MATCH (s) {                                      /* type-safe match (CERIVE_MATCH) */
    CASE (Point, p) { use(p->x); }   /* p is `Point const *`, bound from s */
    CASE (Frame, f) { use(f->id); }  /* wrong-field access won't compile */
}
```

`MATCH`/`CASE` are short aliases for `CERIVE_MATCH`/`CERIVE_CASE` (suppress with
`-DCERIVE_NO_SHORT_NAMES`).

## Layout

```
include/cerive/           the library (header-only, #include <cerive/cerive.h>):
  cerive.h                  per-field generators + CERIVE(T, traits...) combinator
  field.h                   field-kind inference + scalar format registry
  each.h                    CERIVE_P_over: generic comma-list fan-out
  ord.h hash.h new.h        cerive_ordering, FNV-1a, CERIVE_NEW compound-literal
  union.h                   tagged-union helpers (CERIVE_UNION, NEW/IS, MATCH/CASE)
CMakeLists.txt            cerive::cerive INTERFACE lib; Zephyr branch; the bench
zephyr/module.yml Kconfig Zephyr module (header-only; CONFIG_CERIVE adds includes)
flake.nix                 dev shell (toolchain, qemu, cmake/ninja, astyle)
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
variants/<impl>/shapes.h  cerive (derived) | handwritten (baseline), compared
study/study.c             stable entry points so the codegen is emitted
tests/test_shapes.c       correctness + sizing/truncation, run under QEMU per impl
.clangd .vscode/ .envrc   IDE: clangd over build/compile_commands.json
```

## License

MIT — see [LICENSE](LICENSE).
