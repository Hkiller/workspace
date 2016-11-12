#include "dr_pbuf_internal_ops.h"

int dr_pbuf_encode32(uint32_t number, uint8_t buffer[10]) {
	if (number < 0x80) {
		buffer[0] = (uint8_t) number ; 
		return 1;
	}
	buffer[0] = (uint8_t) (number | 0x80 );
	if (number < 0x4000) {
		buffer[1] = (uint8_t) (number >> 7 );
		return 2;
	}
	buffer[1] = (uint8_t) ((number >> 7) | 0x80 );
	if (number < 0x200000) {
		buffer[2] = (uint8_t) (number >> 14);
		return 3;
	}
	buffer[2] = (uint8_t) ((number >> 14) | 0x80 );
	if (number < 0x10000000) {
		buffer[3] = (uint8_t) (number >> 21);
		return 4;
	}
	buffer[3] = (uint8_t) ((number >> 21) | 0x80 );
	buffer[4] = (uint8_t) (number >> 28);
	return 5;
}

int dr_pbuf_encode64(uint64_t number, uint8_t buffer[10]) {
    int i;
	if ((number & 0xffffffff) == number) {
		return dr_pbuf_encode32((uint32_t)number , buffer);
	}
	i = 0;
	do {
		buffer[i] = (uint8_t)(number | 0x80);
		number >>= 7;
		++i;
	} while (number >= 0x80);
	buffer[i] = (uint8_t)number;
	return i+1;
}

int
dr_pbuf_decode(uint8_t const buffer[10], struct dr_pbuf_longlong *result) {
    uint32_t r;
	int i;
    uint64_t lr;

	if (!(buffer[0] & 0x80)) {
		result->low = buffer[0];
		result->hi = 0;
		return 1;
	}
	r = buffer[0] & 0x7f;
	for (i=1;i<4;i++) {
		r |= ((buffer[i]&0x7f) << (7*i));
		if (!(buffer[i] & 0x80)) {
			result->low = r;
			result->hi = 0;
			return i+1;
		}
	}
	lr = 0;
	for (i=4;i<10;i++) {
		lr |= ((buffer[i] & 0x7f) << (7*(i-4)));
		if (!(buffer[i] & 0x80)) {
			result->hi = (uint32_t)(lr >> 4);
			result->low = r | (((uint32_t)lr & 0xf) << 28);
			return i+1;
		}
	}

	result->low = 0;
	result->hi = 0;
	return 0;
}

int dr_pbuf_zigzag32(int32_t n, uint8_t buffer[10]) {
	n = (n << 1) ^ (n >> 31);
	return dr_pbuf_encode32(n,buffer);
}

int dr_pbuf_zigzag64(int64_t n, uint8_t buffer[10]) {
	n = (n << 1) ^ (n >> 63);
	return dr_pbuf_encode64(n,buffer);
}

void dr_pbuf_dezigzag64(struct dr_pbuf_longlong *r) {
	uint32_t low = r->low;
	r->low = ((low >> 1) | ((r->hi & 1) << 31)) ^ - (low & 1);
	r->hi = (r->hi >> 1) ^ - (low & 1);
}

void dr_pbuf_dezigzag32(struct dr_pbuf_longlong *r) {
	uint32_t low = r->low;
	r->low = (low >> 1) ^ - (low & 1);
	r->hi = -(low >> 31);
}

int dr_pbuf_decode_uint32(uint8_t buffer[10], uint32_t * number) {
    union {
        struct dr_pbuf_longlong sep;
        uint64_t u64;
    } buf;
    int use_size;

    use_size = dr_pbuf_decode(buffer, &buf.sep);
    if (use_size < 0) return use_size;

    if (number) *number = (uint32_t)buf.u64;

    return use_size;
}

int dr_pbuf_decode_uint64(uint8_t buffer[10], uint64_t * number) {
    union {
        struct dr_pbuf_longlong sep;
        uint64_t u64;
    } buf;
    int use_size;

    use_size = dr_pbuf_decode(buffer, &buf.sep);
    if (use_size < 0) return use_size;

    if (number) *number = buf.u64;

    return use_size;
}

int dr_pbuf_decode_int32(uint8_t buffer[10], int32_t * number) {
    union {
        struct dr_pbuf_longlong sep;
        int64_t i64;
    } buf;
    int use_size;

    use_size = dr_pbuf_decode(buffer, &buf.sep);
    if (use_size < 0) return use_size;

    dr_pbuf_dezigzag64(&buf.sep);

    if (number) *number = (int32_t)buf.i64;

    return use_size;
}

int dr_pbuf_decode_int64(uint8_t buffer[10], int64_t * number) {
    union {
        struct dr_pbuf_longlong sep;
        int64_t i64;
    } buf;
    int use_size;

    use_size = dr_pbuf_decode(buffer, &buf.sep);
    if (use_size < 0) return use_size;

    dr_pbuf_dezigzag64(&buf.sep);

    if (number) *number = buf.i64;

    return use_size;
}

