#ifndef UI_CACHE_PIXEL_FORMAT_H
#define UI_CACHE_PIXEL_FORMAT_H
#include "ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ui_cache_pixel_format_to_stride(ui_cache_pixel_format_t format);
int8_t ui_cache_pixel_format_compressed(ui_cache_pixel_format_t format);
uint8_t ui_cache_pixel_format_is_alpha_zero(ui_cache_pixel_format_t format, void const * data);
const char * ui_cache_pixel_format_name(ui_cache_pixel_format_t format);
    
#ifdef __cplusplus
}
#endif

#endif

