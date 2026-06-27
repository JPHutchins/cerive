#pragma once

/*
 * Fluent construction. NEW(T, ...) wraps a compound literal, so a value is built
 * with (possibly nested) designated initializers and omitted fields zero out:
 *
 *     Frame const f = NEW(Frame, .edge = {.a = {1, 2}, .b = {3, 4}}, .id = 7);
 *
 * It is the compound literal -- same codegen as the generated positional T_new(),
 * which remains for positional construction.
 */

#define NEW(T, ...) (T){__VA_ARGS__}
