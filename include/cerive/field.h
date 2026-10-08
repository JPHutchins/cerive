#pragma once

#include <inttypes.h>

#include "each.h"

#define CERIVE_P_scalar(format, type) format, 1, type,
#define CERIVE_P_scalar_bool CERIVE_P_scalar("%d", bool)
#define CERIVE_P_scalar_char CERIVE_P_scalar("%c", char)
#define CERIVE_P_scalar_int8_t CERIVE_P_scalar("%" PRId8, int8_t)
#define CERIVE_P_scalar_int16_t CERIVE_P_scalar("%" PRId16, int16_t)
#define CERIVE_P_scalar_int32_t CERIVE_P_scalar("%" PRId32, int32_t)
#define CERIVE_P_scalar_int64_t CERIVE_P_scalar("%" PRId64, int64_t)
#define CERIVE_P_scalar_uint8_t CERIVE_P_scalar("%" PRIu8, uint8_t)
#define CERIVE_P_scalar_uint16_t CERIVE_P_scalar("%" PRIu16, uint16_t)
#define CERIVE_P_scalar_uint32_t CERIVE_P_scalar("%" PRIu32, uint32_t)
#define CERIVE_P_scalar_uint64_t CERIVE_P_scalar("%" PRIu64, uint64_t)
#define CERIVE_P_scalar_size_t CERIVE_P_scalar("%zu", size_t)
#define CERIVE_P_scalar_float CERIVE_P_scalar("%g", float)
#define CERIVE_P_scalar_double CERIVE_P_scalar("%g", double)
#define CERIVE_P_scalar_int CERIVE_P_scalar("%d", int)
#define CERIVE_P_scalar_unsigned CERIVE_P_scalar("%u", unsigned)
#define CERIVE_P_scalar_long CERIVE_P_scalar("%ld", long)
#define CERIVE_P_scalar_unsigned_long CERIVE_P_scalar("%lu", unsigned long)
#define CERIVE_P_scalar_long_long CERIVE_P_scalar("%lld", long long)
#define CERIVE_P_scalar_unsigned_long_long CERIVE_P_scalar("%llu", unsigned long long)

#define CERIVE_P_is_scalar(type) CERIVE_P_is_scalar_(CERIVE_P_scalar_##type, 0)
#define CERIVE_P_is_scalar_(...) CERIVE_P_is_scalar__(__VA_ARGS__)
#define CERIVE_P_is_scalar__(format, flag, ...) flag
#define CERIVE_P_scalar_type(type) CERIVE_P_scalar_type_(CERIVE_P_scalar_##type)
#define CERIVE_P_scalar_type_(...) CERIVE_P_scalar_type__(__VA_ARGS__)
#define CERIVE_P_scalar_type__(format, flag, registered, ...) registered
#define CERIVE_P_scalar_format(type) CERIVE_P_scalar_format_(CERIVE_P_scalar_##type)
#define CERIVE_P_scalar_format_(...) CERIVE_P_scalar_format__(__VA_ARGS__)
#define CERIVE_P_scalar_format__(format, ...) format

#define const_CERIVE_P_unconst
#define CERIVE_P_strip_const(type) CERIVE_P_cat(type, _CERIVE_P_unconst)
#define const_CERIVE_P_probe ~, 1,
#define CERIVE_P_is_const(type) CERIVE_P_is_const_(CERIVE_P_cat(type, _CERIVE_P_probe), 0)
#define CERIVE_P_is_const_(...) CERIVE_P_is_const__(__VA_ARGS__)
#define CERIVE_P_is_const__(head, flag, ...) flag
#define CERIVE_P_via_record(handler, type, name) CERIVE_P_via_record_( \
	handler, \
	CERIVE_P_strip_const(type), \
	name \
)
#define CERIVE_P_via_record_(handler, base, name) handler(base, name)

#define CERIVE_P_field_arity(...) CERIVE_P_field_arity_(__VA_ARGS__, 3, 2, 1, 0)
#define CERIVE_P_field_arity_(a, b, c, n, ...) n
#define CERIVE_P_field_kind(...) CERIVE_P_field_kind_n( \
	CERIVE_P_field_arity(__VA_ARGS__), \
	__VA_ARGS__ \
)
#define CERIVE_P_field_kind_n(n, ...) CERIVE_P_field_kind_n_(n, __VA_ARGS__)
#define CERIVE_P_field_kind_n_(n, ...) CERIVE_P_field_kind_##n(__VA_ARGS__)
#define CERIVE_P_field_kind_3(star, type, name) pointer
#define CERIVE_P_field_kind_2(type, name) CERIVE_P_field_kind_value(CERIVE_P_is_scalar(type), type)
#define CERIVE_P_field_kind_value(is_scalar, type) CERIVE_P_field_kind_value_(is_scalar, type)
#define CERIVE_P_field_kind_value_(is_scalar, type) CERIVE_P_field_kind_s##is_scalar(type)
#define CERIVE_P_field_kind_s1(type) scalar
#define CERIVE_P_field_kind_s0(type) CERIVE_P_field_kind_record(CERIVE_P_is_const(type))
#define CERIVE_P_field_kind_record(is_const) CERIVE_P_field_kind_record_(is_const)
#define CERIVE_P_field_kind_record_(is_const) CERIVE_P_field_kind_c##is_const
#define CERIVE_P_field_kind_c1 const_record
#define CERIVE_P_field_kind_c0 record

#define CERIVE_P_dispatch(prefix, ...) CERIVE_P_dispatch_( \
	prefix, \
	CERIVE_P_field_kind(__VA_ARGS__), \
	__VA_ARGS__ \
)
#define CERIVE_P_dispatch_(prefix, kind, ...) CERIVE_P_dispatch__(prefix, kind, __VA_ARGS__)
#define CERIVE_P_dispatch__(prefix, kind, ...) prefix##_##kind(__VA_ARGS__)
