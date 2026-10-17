#include <string.h>
#include <cerive/cerive.h>

struct cbor_writer { uint8_t * at; uint8_t const * end; bool ok; };
struct cbor_reader { uint8_t const * at; uint8_t const * end; bool ok; };
struct cbor_text { uint8_t const * at; uint64_t len; };
enum cbor_major : uint8_t {
	cbor_uint = 0,
	cbor_nint = 1,
	cbor_tstr = 3,
	cbor_map = 5,
	cbor_simple = 7,
};

#define CBOR_HEAD_SIZE(n) ( \
	(n) < 24 ? 1 : \
	(n) <= 0xff ? 2 : \
	(n) <= 0xffff ? 3 : \
	(n) <= 0xffffffff ? 5 : \
	9 \
)

static void cbor_put_byte(struct cbor_writer * const w, uint8_t const b) {
	w->ok &= w->at < w->end;
	if (w->ok) { *w->at++ = b; }
}
static void cbor_put_be(struct cbor_writer * const w, uint64_t const v, int const bytes) {
	for (int shift = (bytes - 1) * 8; shift >= 0; shift -= 8) { cbor_put_byte(w, v >> shift); }
}
static void cbor_put_head(
	struct cbor_writer * const w,
	enum cbor_major const major,
	uint64_t const arg
) {
	int const info = (
		arg < 24 ? arg :
		arg <= 0xff ? 24 :
		arg <= 0xffff ? 25 :
		arg <= 0xffffffff ? 26 :
		27
	);
	cbor_put_byte(w, major << 5 | info);
	if (info >= 24) { cbor_put_be(w, arg, 1 << (info - 24)); }
}
static void cbor_put_u64(struct cbor_writer * const w, uint64_t const v) {
	cbor_put_head(w, cbor_uint, v);
}
static void cbor_put_i64(struct cbor_writer * const w, int64_t const v) {
	if (v >= 0) { cbor_put_head(w, cbor_uint, v); } else { cbor_put_head(w, cbor_nint, -1 - v); }
}
static void cbor_put_bool(struct cbor_writer * const w, bool const v) {
	cbor_put_byte(w, 0xf4 + v);
}
static void cbor_put_f32(struct cbor_writer * const w, float const v) {
	uint32_t bits;
	memcpy(&bits, &v, sizeof bits);
	cbor_put_byte(w, 0xfa);
	cbor_put_be(w, bits, 4);
}
static void cbor_put_text(
	struct cbor_writer * const w,
	size_t const len,
	char const text[static len]
) {
	cbor_put_head(w, cbor_tstr, len);
	for (size_t i = 0; i < len; ++i) { cbor_put_byte(w, text[i]); }
}
#define cbor_put(w, value) _Generic((value), \
	int8_t: cbor_put_i64, int16_t: cbor_put_i64, int32_t: cbor_put_i64, int64_t: cbor_put_i64, \
	uint8_t: cbor_put_u64, uint16_t: cbor_put_u64, uint32_t: cbor_put_u64, uint64_t: cbor_put_u64, \
	bool: cbor_put_bool, float: cbor_put_f32)(w, value)
#define CBOR_MAX_SIZE(type) _Generic( \
	(type){}, \
	int8_t: 2, \
	int16_t: 3, \
	int32_t: 5, \
	int64_t: 9, \
	uint8_t: 2, \
	uint16_t: 3, \
	uint32_t: 5, \
	uint64_t: 9, \
	bool: 1, \
	float: 5 \
)

static uint8_t cbor_next(struct cbor_reader * const r) {
	r->ok &= r->at < r->end;
	return r->ok ? *r->at++ : 0;
}
static uint64_t cbor_get_head(struct cbor_reader * const r, enum cbor_major const major) {
	uint8_t const initial = cbor_next(r);
	uint8_t const info = initial & 0x1f;
	r->ok &= initial >> 5 == major && info < 28;
	uint64_t arg = info < 24 ? info : 0;
	for (int i = 0; r->ok && info >= 24 && i < 1 << (info - 24); ++i) { arg = (
		arg << 8 |
		cbor_next(r)
	); }
	return r->ok ? arg : 0;
}
static uint64_t cbor_get_uint64_t(struct cbor_reader * const r) {
	return cbor_get_head(r, cbor_uint);
}
static int64_t cbor_get_int64_t(struct cbor_reader * const r) {
	bool const negative = r->ok && r->at < r->end && *r->at >> 5 == cbor_nint;
	uint64_t const arg = cbor_get_head(r, negative ? cbor_nint : cbor_uint);
	r->ok &= arg <= INT64_MAX;
	int64_t const magnitude = r->ok ? arg : 0;
	return negative ? -1 - magnitude : magnitude;
}
#define CBOR_GET_UNSIGNED(type, max) \
	static type cbor_get_##type(struct cbor_reader * const r) { \
		uint64_t const v = cbor_get_uint64_t(r); \
		r->ok &= v <= max; \
		return r->ok ? v : 0; \
	}
#define CBOR_GET_SIGNED(type, min, max) \
	static type cbor_get_##type(struct cbor_reader * const r) { \
		int64_t const v = cbor_get_int64_t(r); \
		r->ok &= v >= min && v <= max; \
		return r->ok ? v : 0; \
	}
CBOR_GET_UNSIGNED(uint8_t, UINT8_MAX)
CBOR_GET_UNSIGNED(uint16_t, UINT16_MAX)
CBOR_GET_UNSIGNED(uint32_t, UINT32_MAX)
CBOR_GET_SIGNED(int8_t, INT8_MIN, INT8_MAX)
CBOR_GET_SIGNED(int16_t, INT16_MIN, INT16_MAX)
CBOR_GET_SIGNED(int32_t, INT32_MIN, INT32_MAX)
static bool cbor_get_bool(struct cbor_reader * const r) {
	uint8_t const b = cbor_next(r);
	r->ok &= b == 0xf4 || b == 0xf5;
	return b == 0xf5;
}
static float cbor_get_f32(struct cbor_reader * const r) {
	r->ok &= cbor_next(r) == 0xfa;
	uint32_t bits = 0;
	for (int i = 0; r->ok && i < 4; ++i) { bits = bits << 8 | cbor_next(r); }
	float v;
	memcpy(&v, &bits, sizeof v);
	return r->ok ? v : 0.0f;
}
static struct cbor_text cbor_get_text(struct cbor_reader * const r) {
	uint64_t const len = cbor_get_head(r, cbor_tstr);
	size_t const remaining = r->end - r->at;
	r->ok &= len <= remaining;
	struct cbor_text const text = {.at = r->at, .len = r->ok ? len : 0};
	r->at += text.len;
	return text;
}
static bool cbor_text_is(
	struct cbor_text const text,
	size_t const len,
	char const name[static len]
) {
	return text.len == len && memcmp(text.at, name, len) == 0;
}
#define cbor_get(r, lvalue) _Generic((lvalue), \
	int8_t: cbor_get_int8_t, int16_t: cbor_get_int16_t, int32_t: cbor_get_int32_t, int64_t: cbor_get_int64_t, \
	uint8_t: cbor_get_uint8_t, uint16_t: cbor_get_uint16_t, uint32_t: cbor_get_uint32_t, \
	uint64_t: cbor_get_uint64_t, bool: cbor_get_bool, float: cbor_get_f32)(r)

#define CBOR_KEY_SIZE(name) (CBOR_HEAD_SIZE(sizeof #name - 1) + sizeof #name - 1)
#define CBOR_COUNT(...) + 1
#define CBOR_SIZE(...) CERIVE_P_dispatch(CBOR_SIZE, __VA_ARGS__)
#define CBOR_SIZE_scalar(type, name) + CBOR_KEY_SIZE(name) + CBOR_MAX_SIZE(type)
#define CBOR_SIZE_record(type, name) + CBOR_KEY_SIZE(name) + type##_cbor_max_size
#define CBOR_SIZE_const_record(type, name) CERIVE_P_via_record(CBOR_SIZE_record, type, name)
#define CBOR_SIZE_pointer(star, type, name) + sizeof(struct { static_assert(0, #name ": no CBOR form"); int _; })
#define CBOR_ENC(...) CERIVE_P_dispatch(CBOR_ENC, __VA_ARGS__)
#define CBOR_ENC_scalar(type, name) cbor_put_text(w, sizeof #name - 1, #name); cbor_put(w, self->name);
#define CBOR_ENC_record(type, name) cbor_put_text(w, sizeof #name - 1, #name); type##_cbor_encode(&self->name, w);
#define CBOR_ENC_const_record(type, name) CERIVE_P_via_record(CBOR_ENC_record, type, name)
#define CBOR_ENC_pointer(star, type, name) static_assert(0, #name ": no CBOR form");
#define CBOR_INDEX(...) CERIVE_P_dispatch(CBOR_INDEX, __VA_ARGS__)
#define CBOR_INDEX_scalar(type, name) cbor_field_##name,
#define CBOR_INDEX_record(type, name) cbor_field_##name,
#define CBOR_INDEX_const_record(type, name) cbor_field_##name,
#define CBOR_INDEX_pointer(star, type, name) cbor_field_##name,
#define CBOR_LOCAL(...) CERIVE_P_dispatch(CBOR_LOCAL, __VA_ARGS__)
#define CBOR_LOCAL_scalar(type, name) type field_##name = {};
#define CBOR_LOCAL_record(type, name) type field_##name = {};
#define CBOR_LOCAL_const_record(type, name) CERIVE_P_via_record(CBOR_LOCAL_record, type, name)
#define CBOR_LOCAL_pointer(star, type, name) static_assert(0, #name ": no CBOR form");
#define CBOR_DEC(...) CERIVE_P_dispatch(CBOR_DEC, __VA_ARGS__)
#define CBOR_DEC_field(name, decode) \
	if (cbor_text_is(key, sizeof #name - 1, #name) && !(seen & (1u << cbor_field_##name))) { \
		seen |= 1u << cbor_field_##name; \
		field_##name = decode; \
		continue; \
	}
#define CBOR_DEC_scalar(type, name) CBOR_DEC_field(name, cbor_get(r, field_##name))
#define CBOR_DEC_record(type, name) CBOR_DEC_field(name, type##_cbor_decode(r))
#define CBOR_DEC_const_record(type, name) CERIVE_P_via_record(CBOR_DEC_record, type, name)
#define CBOR_DEC_pointer(star, type, name)
#define CBOR_INIT(...) CERIVE_P_dispatch(CBOR_INIT, __VA_ARGS__)
#define CBOR_INIT_scalar(type, name) .name = field_##name,
#define CBOR_INIT_record(type, name) .name = field_##name,
#define CBOR_INIT_const_record(type, name) .name = field_##name,
#define CBOR_INIT_pointer(star, type, name)
#define CERIVE_Cbor(T) \
	enum : size_t { \
		T##_cbor_field_count = 0 T##_FIELDS(CBOR_COUNT), \
		T##_cbor_max_size = CBOR_HEAD_SIZE(T##_cbor_field_count) T##_FIELDS(CBOR_SIZE), \
	}; \
	static_assert(T##_cbor_field_count < 32); \
	[[maybe_unused]] static void T##_cbor_encode(T const * const self, struct cbor_writer * const w) { \
		cbor_put_head(w, cbor_map, T##_cbor_field_count); \
		T##_FIELDS(CBOR_ENC) \
	} \
	[[maybe_unused]] static T T##_cbor_decode(struct cbor_reader * const r) { \
		enum { T##_FIELDS(CBOR_INDEX) }; \
		T##_FIELDS(CBOR_LOCAL) \
		uint32_t seen = 0; \
		uint64_t const entries = cbor_get_head(r, cbor_map); \
		for (uint64_t i = 0; r->ok && i < entries; ++i) { \
			struct cbor_text const key = cbor_get_text(r); \
			T##_FIELDS(CBOR_DEC) \
			r->ok = false; \
		} \
		r->ok &= seen == (1u << T##_cbor_field_count) - 1; \
		return (T){T##_FIELDS(CBOR_INIT)}; \
	}

#define Sample_FIELDS(X) X(uint8_t, id) X(int32_t, value)
CERIVE(Sample, Struct, PartialEq, Cbor)
#define Frame_FIELDS(X) X(Sample, a) X(Sample const, b) X(uint16_t, crc) X(bool, valid) X(float, gain)
CERIVE(Frame, Struct, PartialEq, Cbor)
#define FrameReordered_FIELDS(X) X(float, gain) X(uint16_t, crc) X(Sample, b) X(bool, valid) X(Sample, a)
CERIVE(FrameReordered, Struct, Cbor)
#define SampleWide_FIELDS(X) X(uint32_t, id) X(int32_t, value)
CERIVE(SampleWide, Struct, Cbor)
#define SampleMissing_FIELDS(X) X(uint8_t, id)
CERIVE(SampleMissing, Struct, Cbor)
#define SampleExtra_FIELDS(X) X(uint8_t, id) X(int32_t, value) X(uint8_t, extra)
CERIVE(SampleExtra, Struct, Cbor)

static struct cbor_reader encode_frame(
	Frame const * const f,
	size_t const n,
	uint8_t buf[static n]
) {
	struct cbor_writer w = {.at = buf, .end = buf + n, .ok = true};
	Frame_cbor_encode(f, &w);
	return (struct cbor_reader){.at = buf, .end = w.at, .ok = w.ok};
}

int main(void) {
	Frame const f = {.a = {7, -300}, .b = {9, 70000}, .crc = 0xBEEF, .valid = true, .gain = 1.5f};
	uint8_t buf[Frame_cbor_max_size];
	struct cbor_reader r = encode_frame(&f, sizeof buf, buf);
	printf("max_size=%zu encoded=%td ok=%d\nhex=", (size_t)Frame_cbor_max_size, r.end - r.at, r.ok);
	for (uint8_t const * p = r.at; p < r.end; ++p) { printf("%02x", *p); }
	Frame const back = Frame_cbor_decode(&r);
	printf("\nround_trip=%d\n", r.ok && r.at == r.end && Frame_eq(&f, &back));

	FrameReordered const swapped = {
		.gain = 1.5f,
		.crc = 0xBEEF,
		.b = {9, 70000},
		.valid = true,
		.a = {7, -300},
	};
	struct cbor_writer w = {.at = buf, .end = buf + sizeof buf, .ok = true};
	FrameReordered_cbor_encode(&swapped, &w);
	r = (struct cbor_reader){.at = buf, .end = w.at, .ok = w.ok};
	Frame const from_swapped = Frame_cbor_decode(&r);
	printf("any_key_order=%d\n", r.ok && Frame_eq(&f, &from_swapped));

	w = (struct cbor_writer){.at = buf, .end = buf + sizeof buf, .ok = true};
	SampleWide_cbor_encode(&(SampleWide){.id = 300, .value = 1}, &w);
	r = (struct cbor_reader){.at = buf, .end = w.at, .ok = w.ok};
	Sample_cbor_decode(&r);
	printf("rejects_300_into_uint8=%d\n", !r.ok);

	w = (struct cbor_writer){.at = buf, .end = buf + sizeof buf, .ok = true};
	SampleMissing_cbor_encode(&(SampleMissing){.id = 1}, &w);
	r = (struct cbor_reader){.at = buf, .end = w.at, .ok = w.ok};
	Sample_cbor_decode(&r);
	printf("rejects_missing_key=%d\n", !r.ok);

	r = encode_frame(&f, sizeof buf, buf);
	r.end -= 1;
	Frame_cbor_decode(&r);
	printf("rejects_truncated=%d\n", !r.ok);

	w = (struct cbor_writer){.at = buf, .end = buf + sizeof buf, .ok = true};
	SampleExtra_cbor_encode(&(SampleExtra){.id = 1, .value = 2, .extra = 3}, &w);
	r = (struct cbor_reader){.at = buf, .end = w.at, .ok = w.ok};
	Sample_cbor_decode(&r);
	printf("rejects_unknown_key=%d\n", !r.ok);

	uint8_t const duplicate[] = {
		0xa3,
		0x62,
		'i',
		'd',
		0x01,
		0x65,
		'v',
		'a',
		'l',
		'u',
		'e',
		0x02,
		0x62,
		'i',
		'd',
		0x03,
	};
	r = (struct cbor_reader){.at = duplicate, .end = duplicate + sizeof duplicate, .ok = true};
	Sample_cbor_decode(&r);
	printf("rejects_duplicate_key=%d\n", !r.ok);
}
