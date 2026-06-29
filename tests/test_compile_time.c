/*
 * Compile-time validation file. NOT run under QEMU -- just compiled, so the
 * _Static_assert checks are verified during the build.
 */
#include <stddef.h>
#include <stdint.h>

#include <cerive/cerive.h>

/* A minimal test type for macro expansion sanity checks. */
#define CompileCheck_FIELDS(X) \
	X(int32_t, a) \
	X(int64_t, b)
CERIVE(CompileCheck, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

_Static_assert(CERIVE_VERSION_MAJOR == 0, "version major");
_Static_assert(CERIVE_VERSION_MINOR == 1, "version minor");
_Static_assert(CERIVE_VERSION_PATCH == 0, "version patch");

_Static_assert(cerive_hash_offset != 0, "FNV offset non-zero");
_Static_assert(cerive_hash_prime != 0, "FNV prime non-zero");

_Static_assert(sizeof(int32_t) == 4, "int32_t is 4 bytes");
_Static_assert(sizeof(int64_t) == 8, "int64_t is 8 bytes");
_Static_assert(sizeof(uint8_t) == 1, "uint8_t is 1 byte");
_Static_assert(sizeof(uint16_t) == 2, "uint16_t is 2 bytes");

_Static_assert(sizeof(CompileCheck) == 12
	|| sizeof(CompileCheck) == 16,
	"CompileCheck expected 12 or 16 bytes (int32_t+int64_t, may pad)");
_Static_assert(sizeof(CompileCheck) >= 12, "CompileCheck at least 12 bytes");

/* Macro expansion sanity: verify the functions are callable. */
__attribute__((used)) static void sanity(void) {
	CompileCheck const x = CompileCheck_default();
	CompileCheck const y = CompileCheck_new(1, 2);
	(void) CompileCheck_eq(&x, &y);
	(void) CompileCheck_cmp(&x, &y);
	(void) CompileCheck_hash(&x);
	char buf[64];
	(void) CompileCheck_debug(&x, buf, sizeof buf);
}
