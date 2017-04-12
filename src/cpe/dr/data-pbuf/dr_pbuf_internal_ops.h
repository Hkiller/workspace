#ifndef CPE_DR_PBUF_INTERNAL_OPS_H
#define CPE_DR_PBUF_INTERNAL_OPS_H
#include "dr_pbuf_internal_types.h"

struct dr_pbuf_longlong {
	uint32_t low;
	uint32_t hi;
};

int dr_pbuf_decode(uint8_t const buffer[10], struct dr_pbuf_longlong *result);
void dr_pbuf_dezigzag64(struct dr_pbuf_longlong *r);
void dr_pbuf_dezigzag32(struct dr_pbuf_longlong*r);


#endif
