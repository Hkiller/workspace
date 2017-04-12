#ifndef UI_SPRITE_WORLDR_ATTR_H
#define UI_SPRITE_WORLDR_ATTR_H
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dr_data_entry_t ui_sprite_world_find_attr(dr_data_entry_t buff, ui_sprite_world_t world, const char * path);

int ui_sprite_world_set_attr(
    ui_sprite_world_t world, const char * path, const char * value, dr_data_source_t data_source);

int ui_sprite_world_bulk_set_attrs(
    ui_sprite_world_t world, const char * defs, dr_data_source_t data_source);

dr_data_source_t ui_sprite_world_build_data_source(ui_sprite_world_t world, dr_data_source_t data_source_buf, uint16_t buf_capacity);

int8_t ui_sprite_world_get_attr_int8(ui_sprite_world_t world, const char * path, int8_t dft);
uint8_t ui_sprite_world_get_attr_uint8(ui_sprite_world_t world, const char * path, uint8_t dft);
int16_t ui_sprite_world_get_attr_int16(ui_sprite_world_t world, const char * path, int16_t dft);
uint16_t ui_sprite_world_get_attr_uint16(ui_sprite_world_t world, const char * path, uint16_t dft);
int32_t ui_sprite_world_get_attr_int32(ui_sprite_world_t world, const char * path, int32_t dft);
uint32_t ui_sprite_world_get_attr_uint32(ui_sprite_world_t world, const char * path, uint32_t dft);
int64_t ui_sprite_world_get_attr_int64(ui_sprite_world_t world, const char * path, int64_t dft);
uint64_t ui_sprite_world_get_attr_uint64(ui_sprite_world_t world, const char * path, uint64_t dft);
float ui_sprite_world_get_attr_float(ui_sprite_world_t world, const char * path, float dft);
double ui_sprite_world_get_attr_double(ui_sprite_world_t world, const char * path, double dft);
const char * ui_sprite_world_get_attr_string(ui_sprite_world_t world, const char * path, const char * dft);

int ui_sprite_world_set_attr_int8(ui_sprite_world_t world, const char * path, int8_t v);
int ui_sprite_world_set_attr_uint8(ui_sprite_world_t world, const char * path, uint8_t v);
int ui_sprite_world_set_attr_int16(ui_sprite_world_t world, const char * path, int16_t v);
int ui_sprite_world_set_attr_uint16(ui_sprite_world_t world, const char * path, uint16_t v);
int ui_sprite_world_set_attr_int32(ui_sprite_world_t world, const char * path, int32_t v);
int ui_sprite_world_set_attr_uint32(ui_sprite_world_t world, const char * path, uint32_t v);
int ui_sprite_world_set_attr_int64(ui_sprite_world_t world, const char * path, int64_t v);
int ui_sprite_world_set_attr_uint64(ui_sprite_world_t world, const char * path, uint64_t v);
int ui_sprite_world_set_attr_float(ui_sprite_world_t world, const char * path, float v);
int ui_sprite_world_set_attr_double(ui_sprite_world_t world, const char * path, double v);
int ui_sprite_world_set_attr_string(ui_sprite_world_t world, const char * path, const char * v);
int ui_sprite_world_set_attr_bin(ui_sprite_world_t world, const char * path, const void * v, LPDRMETA meta);

int ui_sprite_world_set_attr_value(ui_sprite_world_t world, const char * path, dr_value_t value);

#ifdef __cplusplus
}
#endif

#endif
