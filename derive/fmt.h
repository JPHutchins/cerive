#pragma once

#include <inttypes.h>

/*
 * Compile-time format registry. A SCALAR field declares only its type and name;
 * Debug pastes DERIVE_FMT_##type to recover the printf conversion. Because each
 * entry is a string literal, it folds into the field's format at compile time
 * (one snprintf, no runtime lookup). The type must be a single token (the
 * stdint fixed-width family, bool, char, size_t -- not `unsigned int`); an
 * unregistered type is a compile error at the Debug expansion.
 */

#define DERIVE_FMT_bool "%d"
#define DERIVE_FMT_char "%c"
#define DERIVE_FMT_int8_t "%" PRId8
#define DERIVE_FMT_int16_t "%" PRId16
#define DERIVE_FMT_int32_t "%" PRId32
#define DERIVE_FMT_int64_t "%" PRId64
#define DERIVE_FMT_uint8_t "%" PRIu8
#define DERIVE_FMT_uint16_t "%" PRIu16
#define DERIVE_FMT_uint32_t "%" PRIu32
#define DERIVE_FMT_uint64_t "%" PRIu64
#define DERIVE_FMT_size_t "%zu"
