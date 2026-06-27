#pragma once

/*
 * Fluent construction. CERIVE_NEW(T, ...) wraps a compound literal, so a value is
 * built with (possibly nested) designated initializers and omitted fields zero:
 *
 *     Frame const f = CERIVE_NEW(Frame, .edge = {.a = {1, 2}, .b = {3, 4}}, .id = 7);
 *
 * It is the compound literal -- same codegen as the generated positional <T>_new()
 * (the Constructor trait), which remains for positional construction.
 */

#define CERIVE_NEW(T, ...) (T){__VA_ARGS__}
