#include <stdio.h>
#include <string.h>

#include "check.h"
#include "test_types.h"

static int nested_struct(void) {
	int fails = 0;

	Frame const f = CERIVE_NEW(Frame, .edge = {.a = {1, 2}, .b = {3, 4}}, .id = 7);
	int const need = Frame_debug_len(&f);

	char all[128];
	CHECK(Frame_debug(&f, sizeof all, all) == need);
	CHECK(all[need] == '\0' && all[need - 1] == '}');

	char one[1];
	CHECK(Frame_debug(&f, sizeof one, one) == need);
	CHECK(one[0] == '\0');

	char part[12];
	CHECK(Frame_debug(&f, sizeof part, part) == need);
	CHECK(part[sizeof part - 1] == '\0');
	CHECK(memcmp(part, all, sizeof part - 1) == 0);

	return fails;
}

static int tagged_union(void) {
	int fails = 0;

	Shape const s = Shape_new(Line, .a = Point_new(1, 2), .b = Point_new(3, 4));
	int const need = Shape_debug_len(&s);

	char one[1];
	CHECK(Shape_debug(&s, sizeof one, one) == need);
	CHECK(one[0] == '\0');

	char part[9];
	CHECK(Shape_debug(&s, sizeof part, part) == need);
	CHECK(part[sizeof part - 1] == '\0');

	return fails;
}

static int no_fields(void) {
	int fails = 0;

	Empty const e = {};
	char one[1];
	CHECK(Empty_debug(&e, sizeof one, one) == Empty_debug_len(&e));
	CHECK(one[0] == '\0');

	return fails;
}

int main(void) {
	int const fails = nested_struct() + tagged_union() + no_fields();
	puts(fails == 0 ? "all tests passed" : "FAILURES");
	return fails;
}
