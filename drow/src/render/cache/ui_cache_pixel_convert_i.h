#ifndef UI_CACHE_PIXEL_CONVERT_I_H
#define UI_CACHE_PIXEL_CONVERT_I_H
#include "ui_cache_manager_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_cache_pixel_format_convert_fun_t)(void * o, void const * i, size_t count);

ui_cache_pixel_format_convert_fun_t ui_cache_pixel_find_convert(ui_cache_pixel_format_t to, ui_cache_pixel_format_t from);

#ifdef __cplusplus
}
#endif

#endif
