# cerive

Derived struct methods and tagged unions for embedded C23.

```c
#include <string.h>

#include <cerive/cerive.h>
#include <cerive/match.h>

#define Point_FIELDS(X) \
	X(int32_t, x) \
	X(int32_t, y)
CERIVE(Point, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define Line_FIELDS(X) \
	X(Point, a) \
	X(Point, b)
CERIVE(Line, Struct, Debug, Constructor, PartialEq)

#define Shape_VARIANTS(X) X(Point) X(Line)
CERIVE_UNION(Shape, Debug, PartialEq)
#define Shape_new(...) CERIVE_UNION_NEW(Shape, __VA_ARGS__)

static int32_t right_edge(Shape const shape) {
	MATCH(shape) {
		CASE(Point, p) {
			return p->x;
		}
		CASE(Line, l) {
			return l->a.x > l->b.x ? l->a.x : l->b.x;
		}
	}
	unreachable();
}

int main(void) {
	Line const line = CERIVE_NEW(Line, .a = {1, 2}, .b = {3, 4});
	char text[64];
	Line_debug(&line, text, sizeof text);
	return !(
		Line_eq(&line, &(Line){.a = Point_new(1, 2), .b = Point_new(3, 4)})
		&& Point_cmp(&line.a, &line.b) == cerive_less
		&& right_edge(Shape_new(Line, .a = line.a, .b = line.b)) == 3
		&& strcmp(text, "Line { a=Point { x=1 y=2 } b=Point { x=3 y=4 } }") == 0
	);
}
```

| | |
|---|---|
| consume | [CMakeLists.txt](CMakeLists.txt) · [zephyr/module.yml](zephyr/module.yml) |
| usage | [tests/](tests/) |
| tasks | `camas --list` · [tasks.py](tasks.py) |
| toolchain | [flake.nix](flake.nix) |
| evidence | the job summary of a [CI run](https://github.com/JPHutchins/cerive/actions/workflows/ci.yml?query=branch%3Amain) |

## License

MIT — see [LICENSE](LICENSE).
