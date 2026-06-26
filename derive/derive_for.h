#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "sum.h"

/*
 * FOR_EACH derives. The field list is data -- comma-separated tuples mapped over
 * by a __VA_OPT__ recursion. Fields are tagged SCALAR or STRUCT; struct fields
 * compose by value and Debug / PartialEq recurse into the inner type. Structs
 * are emitted without a typedef (refer to them as `struct T`) so a sum's tag
 * constant can share a member struct's name -- see sum.h.
 *
 * Feature-equal to derive_hybrid.h. FOR_EACH_C yields a comma-separated, no-
 * trailing-comma parameter list (so no DROP1), but the EXPAND nesting caps the
 * field/variant count at the unroll depth (~27 here).
 */

static inline size_t derive_rem(size_t const n, int const off) {
	return (off >= 0 && (size_t) off < n) ? n - (size_t) off : 0;
}
static inline char *derive_at(char *const buf, size_t const n, int const off) {
	return buf + ((off >= 0 && (size_t) off < n) ? (size_t) off : n);
}

#define PARENS ()
#define EXPAND(...) EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__)))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__)))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__)))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(m, ...) __VA_OPT__(EXPAND(FOR_EACH_(m, __VA_ARGS__)))
#define FOR_EACH_(m, a, ...) m(a) __VA_OPT__(FOR_EACH_AGAIN PARENS(m, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_

#define FOR_EACH_C(m, ...) __VA_OPT__(EXPAND(FOR_EACH_C_(m, __VA_ARGS__)))
#define FOR_EACH_C_(m, a, ...) m(a) __VA_OPT__(, FOR_EACH_C_AGAIN PARENS(m, __VA_ARGS__))
#define FOR_EACH_C_AGAIN() FOR_EACH_C_

#define SUM_OVER(m, T) FOR_EACH(m, T##_VARIANTS)

#define FOR_DECL(f) FOR_DECL2 f
#define FOR_DECL2(kind, ...) FOR_DECL_##kind(__VA_ARGS__)
#define FOR_DECL_SCALAR(type, name, fmt) type name;
#define FOR_DECL_STRUCT(type, name) struct type name;
#define DERIVE_STRUCT(T) struct T {FOR_EACH(FOR_DECL, T##_FIELDS)}

#define FOR_PARAM(f) FOR_PARAM2 f
#define FOR_PARAM2(kind, ...) FOR_PARAM_##kind(__VA_ARGS__)
#define FOR_PARAM_SCALAR(type, name, fmt) type const name
#define FOR_PARAM_STRUCT(type, name) struct type const name
#define FOR_INIT(f) FOR_INIT2 f
#define FOR_INIT2(kind, ...) FOR_INIT_##kind(__VA_ARGS__)
#define FOR_INIT_SCALAR(type, name, fmt) .name = name
#define FOR_INIT_STRUCT(type, name) .name = name
#define DERIVE_NEW(T) \
	static inline struct T T##_new(FOR_EACH_C(FOR_PARAM, T##_FIELDS)) { \
		return (struct T){FOR_EACH_C(FOR_INIT, T##_FIELDS)}; \
	}

#define DERIVE_DEFAULT(T) \
	static inline struct T T##_default(void) { return (struct T){}; }

#define FOR_EQ(f) FOR_EQ2 f
#define FOR_EQ2(kind, ...) FOR_EQ_##kind(__VA_ARGS__)
#define FOR_EQ_SCALAR(type, name, fmt) &&a->name == b->name
#define FOR_EQ_STRUCT(type, name) &&type##_eq(&a->name, &b->name)
#define DERIVE_PARTIAL_EQ(T) \
	static inline bool T##_eq(struct T const *const a, struct T const *const b) { \
		return true FOR_EACH(FOR_EQ, T##_FIELDS); \
	}

#define FOR_DBG(f) FOR_DBG2 f
#define FOR_DBG2(kind, ...) FOR_DBG_##kind(__VA_ARGS__)
#define FOR_DBG_SCALAR(type, name, fmt) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "=" fmt " ", self->name);
#define FOR_DBG_STRUCT(type, name) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "="); \
	off += type##_debug(&self->name, derive_at(buf, n, off), derive_rem(n, off)); \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), " ");
#define DERIVE_DEBUG(T) \
	static inline int T##_debug(struct T const *const self, char *const buf, size_t const n) { \
		int off = 0; \
		off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #T " { "); \
		FOR_EACH(FOR_DBG, T##_FIELDS) \
		off += snprintf(derive_at(buf, n, off), derive_rem(n, off), "}"); \
		return off; \
	}
