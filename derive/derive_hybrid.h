#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "sum.h"

/*
 * Hybrid derives. Classic operator-threaded `T##_FIELDS(X)` -- one macro per
 * generator, no field-count ceiling; DROP1 peels the leading comma off the
 * constructor parameter list. Fields are tagged SCALAR or STRUCT; struct fields
 * compose by value and Debug / PartialEq recurse. Structs are emitted without a
 * typedef (refer to them as `struct T`) so a sum's tag constant can share a
 * member struct's name -- see sum.h.
 *
 * Feature-equal to derive_for.h.
 */

static inline size_t derive_rem(size_t const n, int const off) {
	return (off >= 0 && (size_t) off < n) ? n - (size_t) off : 0;
}
static inline char *derive_at(char *const buf, size_t const n, int const off) {
	return buf + ((off >= 0 && (size_t) off < n) ? (size_t) off : n);
}

#define DERIVE_DROP1(...) DERIVE_DROP1_(__VA_ARGS__)
#define DERIVE_DROP1_(first, ...) __VA_ARGS__

#define SUM_OVER(m, T) T##_VARIANTS(m)

#define HY_DECL(kind, ...) HY_DECL_##kind(__VA_ARGS__)
#define HY_DECL_SCALAR(type, name, fmt) type name;
#define HY_DECL_STRUCT(type, name) struct type name;
#define DERIVE_STRUCT(T) struct T {T##_FIELDS(HY_DECL)}

#define HY_PARAM(kind, ...) HY_PARAM_##kind(__VA_ARGS__)
#define HY_PARAM_SCALAR(type, name, fmt) , type const name
#define HY_PARAM_STRUCT(type, name) , struct type const name
#define HY_INIT(kind, ...) HY_INIT_##kind(__VA_ARGS__)
#define HY_INIT_SCALAR(type, name, fmt) .name = name,
#define HY_INIT_STRUCT(type, name) .name = name,
#define DERIVE_NEW(T) \
	static inline struct T T##_new(DERIVE_DROP1(T##_FIELDS(HY_PARAM))) { \
		return (struct T){T##_FIELDS(HY_INIT)}; \
	}

#define DERIVE_DEFAULT(T) \
	static inline struct T T##_default(void) { return (struct T){}; }

#define HY_EQ(kind, ...) HY_EQ_##kind(__VA_ARGS__)
#define HY_EQ_SCALAR(type, name, fmt) &&a->name == b->name
#define HY_EQ_STRUCT(type, name) &&type##_eq(&a->name, &b->name)
#define DERIVE_PARTIAL_EQ(T) \
	static inline bool T##_eq(struct T const *const a, struct T const *const b) { \
		return true T##_FIELDS(HY_EQ); \
	}

#define HY_DBG(kind, ...) HY_DBG_##kind(__VA_ARGS__)
#define HY_DBG_SCALAR(type, name, fmt) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "=" fmt " ", self->name);
#define HY_DBG_STRUCT(type, name) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "="); \
	off += type##_debug(&self->name, derive_at(buf, n, off), derive_rem(n, off)); \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), " ");
#define DERIVE_DEBUG(T) \
	static inline int T##_debug(struct T const *const self, char *const buf, size_t const n) { \
		int off = 0; \
		off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #T " { "); \
		T##_FIELDS(HY_DBG) \
		off += snprintf(derive_at(buf, n, off), derive_rem(n, off), "}"); \
		return off; \
	}
