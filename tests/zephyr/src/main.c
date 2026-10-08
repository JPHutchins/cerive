#include <string.h>

#include <zephyr/ztest.h>

#include "shapes.h"

ZTEST(cerive, test_struct_derives) {
	Point const origin = Point_default();
	Point const p = Point_new(1, 2);
	zassert_true(Point_eq(&p, &(Point){.x = 1, .y = 2}));
	zassert_false(Point_eq(&p, &origin));
	zassert_equal(Point_cmp(&origin, &p), cerive_less);
	zassert_equal(Point_cmp(&p, &p), cerive_equal);
	zassert_equal(Point_hash(&p), Point_hash(&(Point){.x = 1, .y = 2}));
	zassert_not_equal(Point_hash(&p), Point_hash(&origin));
}

ZTEST(cerive, test_debug) {
	static char const expect[] = "Line { a=Point { x=1 y=2 } b=Point { x=3 y=4 } }";
	int const need = sizeof expect - 1;
	Line const line = CERIVE_NEW(Line, .a = {1, 2}, .b = {3, 4});
	char text[sizeof expect];
	zassert_equal(Line_debug_len(&line), need);
	zassert_equal(Line_debug(&line, sizeof text, text), need);
	zassert_str_equal(text, expect);
	char one[1];
	zassert_equal(Line_debug(&line, sizeof one, one), need);
	zassert_equal(one[0], '\0');
}

ZTEST(cerive, test_union_derives) {
	Shape const line = Shape_new(Line, .a = Point_new(1, 2), .b = Point_new(3, 4));
	Shape const same = Shape_new(Line, .a = Point_new(1, 2), .b = Point_new(3, 4));
	Shape const point = Shape_new(Point, .x = 1, .y = 2);
	zassert_true(CERIVE_IS(line, Line));
	zassert_false(CERIVE_IS(point, Line));
	zassert_true(Shape_eq(&line, &same));
	zassert_false(Shape_eq(&line, &point));
	static char const expect[] = "Point { x=1 y=2 }";
	char text[sizeof expect];
	zassert_equal(Shape_debug(&point, sizeof text, text), Shape_debug_len(&point));
	zassert_str_equal(text, expect);
}

ZTEST_SUITE(cerive, NULL, NULL, NULL, NULL, NULL);
