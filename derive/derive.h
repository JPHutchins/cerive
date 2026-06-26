#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/*
 * Emulating Rust's `#[derive(...)]` with X-macros.
 *
 * A deriveable type `T` is described by an object-like macro `T##_FIELDS`
 * expanding to a comma-separated list of field tuples:
 *
 *   #define Point_FIELDS (int32_t, x, "%" PRId32), (int32_t, y, "%" PRId32)
 *
 * Each tuple is (c_type, field_name, printf_conversion). The DERIVE_* macros
 * below map a generator over that list to synthesise the struct and its
 * "methods" -- the analogue of a proc-macro expanding a derive.
 */

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

#define DERIVE_DECL_(type, name, fmt) type name;
#define DERIVE_DECL(field) DERIVE_DECL_ field
#define DERIVE_STRUCT(T) typedef struct T {FOR_EACH(DERIVE_DECL, T##_FIELDS)} T

#define DERIVE_DBG_FMT_(type, name, fmt) #name "=" fmt " "
#define DERIVE_DBG_FMT(field) DERIVE_DBG_FMT_ field
#define DERIVE_DBG_ARG_(type, name, fmt) , self->name
#define DERIVE_DBG_ARG(field) DERIVE_DBG_ARG_ field
#define DERIVE_DEBUG(T)                                                       \
	static inline int T##_debug(T const *const self, char *const buf,         \
								size_t const n) {                                \
		return snprintf(buf, n, #T " { " FOR_EACH(DERIVE_DBG_FMT, T##_FIELDS) \
									"}" FOR_EACH(DERIVE_DBG_ARG, T##_FIELDS));   \
	}

#define DERIVE_PARAM_(type, name, fmt) type name
#define DERIVE_PARAM(field) DERIVE_PARAM_ field
#define DERIVE_INIT_(type, name, fmt) .name = name
#define DERIVE_INIT(field) DERIVE_INIT_ field
#define DERIVE_NEW(T)                                                  \
	static inline T T##_new(FOR_EACH_C(DERIVE_PARAM, T##_FIELDS)) {    \
		return (T){FOR_EACH_C(DERIVE_INIT, T##_FIELDS)};              \
	}

#define DERIVE_DEFAULT(T) \
	static inline T T##_default(void) { return (T){}; }

#define DERIVE_EQ_(type, name, fmt) &&a->name == b->name
#define DERIVE_EQ_CMP(field) DERIVE_EQ_ field
#define DERIVE_EQ(T)                                                  \
	static inline bool T##_eq(T const *const a, T const *const b) {   \
		return true FOR_EACH(DERIVE_EQ_CMP, T##_FIELDS);             \
	}
