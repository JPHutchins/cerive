#pragma once

#include <inttypes.h>

#include "each.h"

/*
 * Field-kind inference. A field is written without a kind tag:
 *
 *     (int32_t, x)        scalar       -- compared/hashed inline, Debug via format
 *     (Point, edge)       record       -- recurses into the inner type's derives
 *     (Point const, edge) const_record -- same, qualifier kept in the declaration
 *     (*, Point, next)    pointer      -- by-address; the `*` is used literally
 *
 * The kind is recovered, not declared. Arity splits pointer (3) from value (2); a
 * value type is a scalar if registered below, a const_record if its type ends in
 * `const`, else a record. The CERIVE_P_scalar_<type> registry does double duty:
 * its presence marks a type as scalar, and its payload is that type's printf
 * format -- one entry, no second list.
 *
 * CERIVE_P_dispatch(prefix, field...) routes a field to the matching
 * prefix##_scalar / _record / _const_record / _pointer handler.
 */

#define CERIVE_P_scalar(format) format, 1,
#define CERIVE_P_scalar_bool CERIVE_P_scalar("%d")
#define CERIVE_P_scalar_char CERIVE_P_scalar("%c")
#define CERIVE_P_scalar_int8_t CERIVE_P_scalar("%" PRId8)
#define CERIVE_P_scalar_int16_t CERIVE_P_scalar("%" PRId16)
#define CERIVE_P_scalar_int32_t CERIVE_P_scalar("%" PRId32)
#define CERIVE_P_scalar_int64_t CERIVE_P_scalar("%" PRId64)
#define CERIVE_P_scalar_uint8_t CERIVE_P_scalar("%" PRIu8)
#define CERIVE_P_scalar_uint16_t CERIVE_P_scalar("%" PRIu16)
#define CERIVE_P_scalar_uint32_t CERIVE_P_scalar("%" PRIu32)
#define CERIVE_P_scalar_uint64_t CERIVE_P_scalar("%" PRIu64)
#define CERIVE_P_scalar_size_t CERIVE_P_scalar("%zu")

#define CERIVE_P_is_scalar(type) CERIVE_P_is_scalar_(CERIVE_P_scalar_##type, 0,)
#define CERIVE_P_is_scalar_(...) CERIVE_P_is_scalar__(__VA_ARGS__)
#define CERIVE_P_is_scalar__(format, flag, ...) flag
#define CERIVE_P_scalar_format(type) CERIVE_P_scalar_format_(CERIVE_P_scalar_##type)
#define CERIVE_P_scalar_format_(...) CERIVE_P_scalar_format__(__VA_ARGS__)
#define CERIVE_P_scalar_format__(format, ...) format

/*
 * Opt-in const record members. A value field whose type ends in `const` --
 * `(Point const, edge)` -> `Point const edge;` -- keeps the qualifier in the
 * declaration, but the trait-fn paste base must drop it (else `Point const##_eq`
 * corrupts). CERIVE_P_strip_const pastes a vanishing `const_cerive_unconst` onto
 * the type's last token, so `Point const` -> `Point` (valid only on a const-ending
 * type, the only place it is used). const-qualified scalars need no help --
 * is_scalar keys on the leading token and the format ignores the rest.
 */
#define const_cerive_unconst
#define CERIVE_P_strip_const(type) CERIVE_P_cat(type, _cerive_unconst)
#define const_cerive_const_probe ~, 1,
#define CERIVE_P_is_const(type) CERIVE_P_is_const_(CERIVE_P_cat(type, _cerive_const_probe), 0,)
#define CERIVE_P_is_const_(...) CERIVE_P_is_const__(__VA_ARGS__)
#define CERIVE_P_is_const__(head, flag, ...) flag
#define CERIVE_P_via_record(handler, type, name) \
	CERIVE_P_via_record_(handler, CERIVE_P_strip_const(type), name)
#define CERIVE_P_via_record_(handler, base, name) handler(base, name)

#define CERIVE_P_field_arity(...) CERIVE_P_field_arity_(__VA_ARGS__, 3, 2, 1, 0)
#define CERIVE_P_field_arity_(a, b, c, n, ...) n
#define CERIVE_P_field_kind(...) \
	CERIVE_P_field_kind_n(CERIVE_P_field_arity(__VA_ARGS__), __VA_ARGS__)
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

#define CERIVE_P_dispatch(prefix, ...) \
	CERIVE_P_dispatch_(prefix, CERIVE_P_field_kind(__VA_ARGS__), __VA_ARGS__)
#define CERIVE_P_dispatch_(prefix, kind, ...) CERIVE_P_dispatch__(prefix, kind, __VA_ARGS__)
#define CERIVE_P_dispatch__(prefix, kind, ...) prefix##_##kind(__VA_ARGS__)
