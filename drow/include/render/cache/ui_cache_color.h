#ifndef UI_CACHE_COLOR_H
#define UI_CACHE_COLOR_H
#include "render/utils/ui_utils_types.h"
#include "ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_cache_set_color(ui_cache_manager_t cache, const char * str_color, ui_color_t color);
int ui_cache_find_color(ui_cache_manager_t cache, const char * str_color, ui_color_t r_color);
void ui_cache_get_color(ui_cache_manager_t cache, const char * str_color, ui_color_t dft_color, ui_color_t r_color);
    
#ifdef __cplusplus
}
#endif

#endif

