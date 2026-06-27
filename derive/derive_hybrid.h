#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "fmt.h"
#include "ord.h"
#include "union.h"

/*
 * Hybrid derives. Classic operator-threaded `T##_FIELDS(X)` -- one macro per
 * generator, no field-count ceiling; DROP1 peels the leading comma off the
 * constructor parameter list. Fields are tagged SCALAR or STRUCT; struct fields
 * compose by value and Debug / PartialEq recurse.
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

#define UNION_OVER(m, T) T##_VARIANTS(m)

#define HY_DECL(kind, ...) HY_DECL_##kind(__VA_ARGS__)
#define HY_DECL_SCALAR(type, name) type name;
#define HY_DECL_STRUCT(type, name) type name;
#define HY_DECL_PTR(type, name) type *name;
#define DERIVE_STRUCT(T) typedef struct T {T##_FIELDS(HY_DECL)} T

#define HY_PARAM(kind, ...) HY_PARAM_##kind(__VA_ARGS__)
#define HY_PARAM_SCALAR(type, name) , type const name
#define HY_PARAM_STRUCT(type, name) , type const name
#define HY_PARAM_PTR(type, name) , type *const name
#define HY_INIT(kind, ...) HY_INIT_##kind(__VA_ARGS__)
#define HY_INIT_SCALAR(type, name) .name = name,
#define HY_INIT_STRUCT(type, name) .name = name,
#define HY_INIT_PTR(type, name) .name = name,
#define DERIVE_NEW(T) \
	static inline T T##_new(DERIVE_DROP1(T##_FIELDS(HY_PARAM))) { \
		return (T){T##_FIELDS(HY_INIT)}; \
	}

#define DERIVE_DEFAULT(T) \
	static inline T T##_default(void) { return (T){}; }

#define HY_EQ(kind, ...) HY_EQ_##kind(__VA_ARGS__)
#define HY_EQ_SCALAR(type, name) &&a->name == b->name
#define HY_EQ_STRUCT(type, name) &&type##_eq(&a->name, &b->name)
#define HY_EQ_PTR(type, name) &&a->name == b->name
#define DERIVE_PARTIAL_EQ(T) \
	static inline bool T##_eq(T const *const a, T const *const b) { \
		return true T##_FIELDS(HY_EQ); \
	}

#define HY_ORD(kind, ...) HY_ORD_##kind(__VA_ARGS__)
#define HY_ORD_SCALAR(type, name) \
	{ \
		enum ordering const o = (a->name > b->name) - (a->name < b->name); \
		if (o != ordering_equal) { \
			return o; \
		} \
	}
#define HY_ORD_STRUCT(type, name) \
	{ \
		enum ordering const o = type##_cmp(&a->name, &b->name); \
		if (o != ordering_equal) { \
			return o; \
		} \
	}
#define HY_ORD_PTR(type, name) \
	{ \
		enum ordering const o = (a->name > b->name) - (a->name < b->name); \
		if (o != ordering_equal) { \
			return o; \
		} \
	}
#define DERIVE_ORD(T) \
	static inline enum ordering T##_cmp(T const *const a, T const *const b) { \
		T##_FIELDS(HY_ORD) \
		return ordering_equal; \
	}

#define HY_DBG(kind, ...) HY_DBG_##kind(__VA_ARGS__)
#define HY_DBG_SCALAR(type, name) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "=" DERIVE_FMT_##type " ", self->name);
#define HY_DBG_STRUCT(type, name) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "="); \
	off += type##_debug(&self->name, derive_at(buf, n, off), derive_rem(n, off)); \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), " ");
#define HY_DBG_PTR(type, name) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "=%p ", (void *) self->name);
#define DERIVE_DEBUG(T) \
	static inline int T##_debug(T const *const self, char *const buf, size_t const n) { \
		int off = 0; \
		off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #T " { "); \
		T##_FIELDS(HY_DBG) \
		off += snprintf(derive_at(buf, n, off), derive_rem(n, off), "}"); \
		return off; \
	}
