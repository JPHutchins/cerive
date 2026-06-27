#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "each.h"
#include "field.h"
#include "hash.h"
#include "ord.h"
#include "union.h"

/*
 * Hybrid derives. Classic operator-threaded `T##_FIELDS(X)` -- one macro per
 * generator, no field-count ceiling; DROP1 peels the leading comma off the
 * constructor parameter list. A field's kind (scalar / struct / pointer) is
 * inferred from the tuple by field.h's DISPATCHV; struct fields compose by value
 * and Debug / PartialEq recurse.
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

#define HY_DECL(...) DISPATCHV(HY_DECL, __VA_ARGS__)
#define HY_DECL_SCALAR(type, name) type name;
#define HY_DECL_STRUCT(type, name) type name;
#define HY_DECL_CONSTSTRUCT(type, name) type name;
#define HY_DECL_PTR(star, type, name) type star name;
#define DERIVE_STRUCT(T) typedef struct T {T##_FIELDS(HY_DECL)} T;

#define HY_PARAM(...) DISPATCHV(HY_PARAM, __VA_ARGS__)
#define HY_PARAM_SCALAR(type, name) , type const name
#define HY_PARAM_STRUCT(type, name) , type const name
#define HY_PARAM_CONSTSTRUCT(type, name) , type name
#define HY_PARAM_PTR(star, type, name) , type star const name
#define HY_INIT(...) DISPATCHV(HY_INIT, __VA_ARGS__)
#define HY_INIT_SCALAR(type, name) .name = name,
#define HY_INIT_STRUCT(type, name) .name = name,
#define HY_INIT_CONSTSTRUCT(type, name) .name = name,
#define HY_INIT_PTR(star, type, name) .name = name,
#define DERIVE_NEW(T) \
	static inline T T##_new(DERIVE_DROP1(T##_FIELDS(HY_PARAM))) { \
		return (T){T##_FIELDS(HY_INIT)}; \
	}

#define DERIVE_DEFAULT(T) \
	static inline T T##_default(void) { return (T){}; }

#define HY_EQ(...) DISPATCHV(HY_EQ, __VA_ARGS__)
#define HY_EQ_SCALAR(type, name) &&a->name == b->name
#define HY_EQ_STRUCT(type, name) &&type##_eq(&a->name, &b->name)
#define HY_EQ_CONSTSTRUCT(type, name) CONST_FWD(HY_EQ_STRUCT, type, name)
#define HY_EQ_PTR(star, type, name) &&a->name == b->name
#define DERIVE_PARTIAL_EQ(T) \
	static inline bool T##_eq(T const *const a, T const *const b) { \
		return true T##_FIELDS(HY_EQ); \
	}

#define HY_ORD(...) DISPATCHV(HY_ORD, __VA_ARGS__)
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
#define HY_ORD_CONSTSTRUCT(type, name) CONST_FWD(HY_ORD_STRUCT, type, name)
#define HY_ORD_PTR(star, type, name) \
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

#define HY_HASH(...) DISPATCHV(HY_HASH, __VA_ARGS__)
#define HY_HASH_SCALAR(type, name) h = hash_bytes(h, &self->name, sizeof self->name);
#define HY_HASH_STRUCT(type, name) h = hash_mix(h, type##_hash(&self->name));
#define HY_HASH_CONSTSTRUCT(type, name) CONST_FWD(HY_HASH_STRUCT, type, name)
#define HY_HASH_PTR(star, type, name) h = hash_bytes(h, &self->name, sizeof self->name);
#define DERIVE_HASH(T) \
	static inline size_t T##_hash(T const *const self) { \
		size_t h = hash_offset; \
		T##_FIELDS(HY_HASH) \
		return h; \
	}

#define HY_DBG(...) DISPATCHV(HY_DBG, __VA_ARGS__)
#define HY_DBG_SCALAR(type, name) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "=" SCALAR_FMT(type) " ", self->name);
#define HY_DBG_STRUCT(type, name) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "="); \
	off += type##_debug(&self->name, derive_at(buf, n, off), derive_rem(n, off)); \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), " ");
#define HY_DBG_CONSTSTRUCT(type, name) CONST_FWD(HY_DBG_STRUCT, type, name)
#define HY_DBG_PTR(star, type, name) \
	off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #name "=%p ", (void *) self->name);
#define DERIVE_DEBUG(T) \
	static inline int T##_debug(T const *const self, char *const buf, size_t const n) { \
		int off = 0; \
		off += snprintf(derive_at(buf, n, off), derive_rem(n, off), #T " { "); \
		T##_FIELDS(HY_DBG) \
		off += snprintf(derive_at(buf, n, off), derive_rem(n, off), "}"); \
		return off; \
	}

/*
 * Combinator: DERIVE(T, traits...) fans out to DERIVE_<trait>(T) for each named
 * trait -- DERIVE(Point, STRUCT, DEBUG, NEW, DEFAULT, PARTIAL_EQ, ORD, HASH).
 * STRUCT must lead (it defines the type the rest reference).
 */
#define DERIVE(T, ...) DERIVE_OVER(DERIVE, T, __VA_ARGS__)
