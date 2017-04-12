#ifndef PLUGIN_BARRAGE_UTILS_I_H
#define PLUGIN_BARRAGE_UTILS_I_H
#include "protocol/plugin/barrage/barrage_info.h"
#include "plugin/barrage/plugin_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

float plugin_barrage_calc_float(BARRAGE_RAND_FLOAT const * v);
uint16_t plugin_barrage_calc_uint16(BARRAGE_RAND_UINT16 const * v);
uint8_t plugin_barrage_calc_uint8(BARRAGE_RAND_UINT8 const * v);
void plugin_barrage_calc_pair(BARRAGE_PAIR * r, BARRAGE_RAND_PAIR const * v);

#ifdef __cplusplus
}
#endif

#endif
