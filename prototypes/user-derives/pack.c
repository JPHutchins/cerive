#include <string.h>
#include <cerive/cerive.h>

#define PACK(...) CERIVE_P_dispatch(PACK, __VA_ARGS__)
#define PACK_scalar(type, name) memcpy(buf + off, &self->name, sizeof self->name); off += sizeof self->name;
#define PACK_record(type, name) off += type##_pack(&self->name, buf + off);
#define PACK_const_record(type, name) CERIVE_P_via_record(PACK_record, type, name)
#define PACK_pointer(star, type, name) static_assert(0, #name ": pointers do not pack");
#define PACKED_SIZE(...) CERIVE_P_dispatch(PACKED_SIZE, __VA_ARGS__)
#define PACKED_SIZE_scalar(type, name) + sizeof(type)
#define PACKED_SIZE_record(type, name) + type##_packed_size
#define PACKED_SIZE_const_record(type, name) CERIVE_P_via_record(PACKED_SIZE_record, type, name)
#define PACKED_SIZE_pointer(star, type, name) + sizeof(struct { static_assert(0, #name ": pointers do not pack"); int _; })
#define CERIVE_Pack(T) \
	enum : size_t { T##_packed_size = 0 T##_FIELDS(PACKED_SIZE) }; \
	static inline size_t T##_pack(T const * const self, uint8_t buf[static T##_packed_size]) { \
		size_t off = 0; \
		T##_FIELDS(PACK) \
		return off; \
	}

#define Sample_FIELDS(X) X(uint8_t, id) X(uint32_t, value)
CERIVE(Sample, Struct, Pack, Ord)
#define Frame_FIELDS(X) X(Sample, a) X(Sample const, b) X(uint16_t, crc)
CERIVE(Frame, Struct, Pack)

int main(void) {
	uint8_t wire[Frame_packed_size];
	Frame const f = {.a = {1, 0x11223344}, .b = {2, 0x55667788}, .crc = 0xBEEF};
	printf(
		"sizeof(Frame)=%zu packed=%zu wrote=%zu\n",
		sizeof(Frame),
		(size_t)Frame_packed_size,
		Frame_pack(&f, wire)
	);
	for (size_t i = 0; i < sizeof wire; i++) printf("%02x ", wire[i]);
	puts("");
}
