#pragma once

#include <inttypes.h>

/*
 * Field-kind inference. A field is written without a kind tag:
 *
 *     (int32_t, x)     scalar  -- value compared/hashed inline, Debug via format
 *     (Point, edge)    struct  -- recurses into the inner type's derives
 *     (*, Point, next)  pointer -- by-address; the `*` is used literally in decl
 *
 * The kind is recovered, not declared. Arity splits pointer (3) from value (2);
 * a scalar is a value type registered below, anything else is a struct. The
 * SCALAR_<type> registry does double duty: its presence marks a type as scalar,
 * and its payload is that type's printf format -- one entry, no second list.
 *
 * DISPATCH (tuple form, FOR_EACH) and DISPATCHV (variadic, classic threading)
 * route a field to a generator's _SCALAR / _STRUCT / _PTR handler by kind.
 */

#define SCALAR_REG(fmt) fmt, 1,
#define SCALAR_bool SCALAR_REG("%d")
#define SCALAR_char SCALAR_REG("%c")
#define SCALAR_int8_t SCALAR_REG("%" PRId8)
#define SCALAR_int16_t SCALAR_REG("%" PRId16)
#define SCALAR_int32_t SCALAR_REG("%" PRId32)
#define SCALAR_int64_t SCALAR_REG("%" PRId64)
#define SCALAR_uint8_t SCALAR_REG("%" PRIu8)
#define SCALAR_uint16_t SCALAR_REG("%" PRIu16)
#define SCALAR_uint32_t SCALAR_REG("%" PRIu32)
#define SCALAR_uint64_t SCALAR_REG("%" PRIu64)
#define SCALAR_size_t SCALAR_REG("%zu")

#define IS_SCALAR(type) IS_SCALAR_(SCALAR_##type, 0,)
#define IS_SCALAR_(...) IS_SCALAR__(__VA_ARGS__)
#define IS_SCALAR__(fmt, flag, ...) flag
#define SCALAR_FMT(type) SCALAR_FMT_(SCALAR_##type)
#define SCALAR_FMT_(...) SCALAR_FMT__(__VA_ARGS__)
#define SCALAR_FMT__(fmt, ...) fmt

/*
 * Opt-in const struct members. A value field whose type ends in `const` --
 * `(Inner const, in)` -> `Inner const in;` -- is a CONSTSTRUCT: the qualifier
 * stays in the declaration, but the trait-fn paste base must drop it (else
 * `Inner const##_eq` corrupts). UNCONST pastes a vanishing `const_DROP` onto the
 * type's last token, so `Inner const` -> `Inner` (and only valid on a const-
 * ending type, which is the only place it is used). const-qualified SCALARS need
 * no help -- IS_SCALAR keys on the leading token and SCALAR_FMT ignores the rest.
 */
#define CAT(a, b) CAT_(a, b)
#define CAT_(a, b) a##b
#define const_DROP
#define UNCONST(type) CAT(type, _DROP)
#define const_ISCONST ~, 1,
#define IS_CONST_TYPE(type) IS_CONST_TYPE_(CAT(type, _ISCONST), 0,)
#define IS_CONST_TYPE_(...) IS_CONST_TYPE__(__VA_ARGS__)
#define IS_CONST_TYPE__(head, flag, ...) flag
#define CONST_FWD(handler, type, name) CONST_FWD_(handler, UNCONST(type), name)
#define CONST_FWD_(handler, base, name) handler(base, name)

#define FIELD_COUNT(...) FIELD_COUNT_(__VA_ARGS__, 3, 2, 1, 0)
#define FIELD_COUNT_(a, b, c, n, ...) n
#define FIELD_KIND(...) FIELD_KIND_C(FIELD_COUNT(__VA_ARGS__), __VA_ARGS__)
#define FIELD_KIND_C(n, ...) FIELD_KIND_C2(n, __VA_ARGS__)
#define FIELD_KIND_C2(n, ...) FIELD_KIND_##n(__VA_ARGS__)
#define FIELD_KIND_3(star, type, name) PTR
#define FIELD_KIND_2(type, name) FIELD_KIND_2S(IS_SCALAR(type), type)
#define FIELD_KIND_2S(is, type) FIELD_KIND_2S_(is, type)
#define FIELD_KIND_2S_(is, type) FIELD_KIND_S##is(type)
#define FIELD_KIND_S1(type) SCALAR
#define FIELD_KIND_S0(type) FIELD_KIND_2C(IS_CONST_TYPE(type))
#define FIELD_KIND_2C(isc) FIELD_KIND_2C_(isc)
#define FIELD_KIND_2C_(isc) FIELD_KIND_C##isc
#define FIELD_KIND_C1 CONSTSTRUCT
#define FIELD_KIND_C0 STRUCT

#define DISPATCH(prefix, f) DISPATCH_(prefix, FIELD_KIND f, f)
#define DISPATCH_(prefix, kind, f) DISPATCH__(prefix, kind, f)
#define DISPATCH__(prefix, kind, f) prefix##_##kind f

#define DISPATCHV(prefix, ...) DISPATCHV_(prefix, FIELD_KIND(__VA_ARGS__), __VA_ARGS__)
#define DISPATCHV_(prefix, kind, ...) DISPATCHV__(prefix, kind, __VA_ARGS__)
#define DISPATCHV__(prefix, kind, ...) prefix##_##kind(__VA_ARGS__)
