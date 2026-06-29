#include <cerive/buf.h>
#include <cerive/hash.h>

/*
 * The shared runtime: the handful of non-generated helpers the generated <T>_debug
 * and <T>_hash call. Compiled once here so multiple translation units share a
 * single copy rather than each carrying a static-inline duplicate. The compiler
 * is still free to inline within this TU; cross-TU calls resolve to these.
 */

size_t cerive_buf_remaining(size_t const cap, int const off) {
	return (off >= 0 && (size_t) off < cap) ? cap - (size_t) off : 0;
}

char * cerive_buf_at(char * const buf, size_t const cap, int const off) {
	return buf + ((off >= 0 && (size_t) off < cap) ? (size_t) off : cap);
}

size_t cerive_hash_bytes(size_t hash, void const * const data, size_t const len) {
	unsigned char const * const bytes = data;
	for (size_t i = 0; i < len; ++i) {
		hash = (hash ^ bytes[i]) * cerive_hash_prime;
	}
	return hash;
}

size_t cerive_hash_mix(size_t const hash, size_t const value) {
	return cerive_hash_bytes(hash, &value, sizeof value);
}
