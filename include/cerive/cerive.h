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
 * cerive -- derive struct methods from one field list, in standard C23.
 *
 *     #define Point_FIELDS(X) \
 *         X(int32_t, x) \
 *         X(int32_t, y)
 *     CERIVE(Point, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)
 *
 * generates the struct typedef plus Point_debug / Point_new / Point_default /
 * Point_eq / Point_cmp / Point_hash. Each generator threads the operator over
 * `T##_FIELDS(X)` (no field-count ceiling) and dispatches per field on its
 * inferred kind (scalar / record / const_record / pointer; see field.h). The
 * generated code is intended to be byte-identical to the equivalent hand-written
 * functions at every optimization level.
 */

#define CERIVE_VERSION_MAJOR 0
#define CERIVE_VERSION_MINOR 1
#define CERIVE_VERSION_PATCH 0

/* Offset-cursor helpers: where the next snprintf writes, and how much room is
 * left, so Debug fills the caller's buffer in one pass and returns the would-be
 * length (the buffer-size contract) without intermediate allocations. */
static inline size_t cerive_buf_remaining(size_t const cap, int const off) {
	return (off >= 0 && (size_t) off < cap) ? cap - (size_t) off : 0;
}
static inline char *cerive_buf_at(char *const buf, size_t const cap, int const off) {
	return buf + ((off >= 0 && (size_t) off < cap) ? (size_t) off : cap);
}

#define CERIVE_P_drop_first(...) CERIVE_P_drop_first_(__VA_ARGS__)
#define CERIVE_P_drop_first_(first, ...) __VA_ARGS__

#define CERIVE_P_decl(...) CERIVE_P_dispatch(CERIVE_P_decl, __VA_ARGS__)
#define CERIVE_P_decl_scalar(type, name) type name;
#define CERIVE_P_decl_record(type, name) type name;
#define CERIVE_P_decl_const_record(type, name) type name;
#define CERIVE_P_decl_pointer(star, type, name) type star name;
#define CERIVE_Struct(T) typedef struct T {T##_FIELDS(CERIVE_P_decl)} T;

#define CERIVE_P_param(...) CERIVE_P_dispatch(CERIVE_P_param, __VA_ARGS__)
#define CERIVE_P_param_scalar(type, name) , type const name
#define CERIVE_P_param_record(type, name) , type const name
#define CERIVE_P_param_const_record(type, name) , type name
#define CERIVE_P_param_pointer(star, type, name) , type star const name
#define CERIVE_P_init(...) CERIVE_P_dispatch(CERIVE_P_init, __VA_ARGS__)
#define CERIVE_P_init_scalar(type, name) .name = name,
#define CERIVE_P_init_record(type, name) .name = name,
#define CERIVE_P_init_const_record(type, name) .name = name,
#define CERIVE_P_init_pointer(star, type, name) .name = name,
#define CERIVE_Constructor(T) \
	static inline T T##_new(CERIVE_P_drop_first(T##_FIELDS(CERIVE_P_param))) { \
		return (T){T##_FIELDS(CERIVE_P_init)}; \
	}

#define CERIVE_Default(T) \
	static inline T T##_default(void) { return (T){}; }

#define CERIVE_P_eq(...) CERIVE_P_dispatch(CERIVE_P_eq, __VA_ARGS__)
#define CERIVE_P_eq_scalar(type, name) &&a->name == b->name
#define CERIVE_P_eq_record(type, name) &&type##_eq(&a->name, &b->name)
#define CERIVE_P_eq_const_record(type, name) CERIVE_P_via_record(CERIVE_P_eq_record, type, name)
#define CERIVE_P_eq_pointer(star, type, name) &&a->name == b->name
#define CERIVE_PartialEq(T) \
	static inline bool T##_eq(T const *const a, T const *const b) { \
		return true T##_FIELDS(CERIVE_P_eq); \
	}

#define CERIVE_P_ord(...) CERIVE_P_dispatch(CERIVE_P_ord, __VA_ARGS__)
#define CERIVE_P_ord_scalar(type, name) \
	{ \
		enum cerive_ordering const order = (a->name > b->name) - (a->name < b->name); \
		if (order != cerive_equal) { \
			return order; \
		} \
	}
#define CERIVE_P_ord_record(type, name) \
	{ \
		enum cerive_ordering const order = type##_cmp(&a->name, &b->name); \
		if (order != cerive_equal) { \
			return order; \
		} \
	}
#define CERIVE_P_ord_const_record(type, name) CERIVE_P_via_record(CERIVE_P_ord_record, type, name)
#define CERIVE_P_ord_pointer(star, type, name) \
	{ \
		enum cerive_ordering const order = (a->name > b->name) - (a->name < b->name); \
		if (order != cerive_equal) { \
			return order; \
		} \
	}
#define CERIVE_Ord(T) \
	static inline enum cerive_ordering T##_cmp(T const *const a, T const *const b) { \
		T##_FIELDS(CERIVE_P_ord) \
		return cerive_equal; \
	}

#define CERIVE_P_hash(...) CERIVE_P_dispatch(CERIVE_P_hash, __VA_ARGS__)
#define CERIVE_P_hash_scalar(type, name) hash = cerive_hash_bytes(hash, &self->name, sizeof self->name);
#define CERIVE_P_hash_record(type, name) hash = cerive_hash_mix(hash, type##_hash(&self->name));
#define CERIVE_P_hash_const_record(type, name) CERIVE_P_via_record(CERIVE_P_hash_record, type, name)
#define CERIVE_P_hash_pointer(star, type, name) hash = cerive_hash_bytes(hash, &self->name, sizeof self->name);
#define CERIVE_Hash(T) \
	static inline size_t T##_hash(T const *const self) { \
		size_t hash = cerive_hash_offset; \
		T##_FIELDS(CERIVE_P_hash) \
		return hash; \
	}

#define CERIVE_P_debug(...) CERIVE_P_dispatch(CERIVE_P_debug, __VA_ARGS__)
#define CERIVE_P_debug_scalar(type, name) \
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), \
		#name "=" CERIVE_P_scalar_format(type) " ", self->name);
#define CERIVE_P_debug_record(type, name) \
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), #name "="); \
	off += type##_debug(&self->name, cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off)); \
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), " ");
#define CERIVE_P_debug_const_record(type, name) CERIVE_P_via_record(CERIVE_P_debug_record, type, name)
#define CERIVE_P_debug_pointer(star, type, name) \
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), \
		#name "=%p ", (void *) self->name);
#define CERIVE_Debug(T) \
	static inline int T##_debug(T const *const self, char *const buf, size_t const n) { \
		int off = 0; \
		off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), #T " { "); \
		T##_FIELDS(CERIVE_P_debug) \
		off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "}"); \
		return off; \
	}

/*
 * Combinator: CERIVE(T, traits...) fans out to CERIVE_<trait>(T) for each named
 * trait -- CERIVE(Point, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash).
 * Struct must lead (it defines the type the rest reference).
 */
#define CERIVE(T, ...) CERIVE_P_over(CERIVE, T, __VA_ARGS__)
