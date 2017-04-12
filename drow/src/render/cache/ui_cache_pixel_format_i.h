#ifndef UI_CACHE_PIXEL_FORMAT_I_H
#define UI_CACHE_PIXEL_FORMAT_I_H
#include "render/cache/ui_cache_pixel_format.h"
#include "ui_cache_manager_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_pixel_format_info {
    const char * name;
    uint8_t stride;
    uint8_t compressed;
    uint8_t alpha_start;
    uint8_t alpha_mask;
};

extern struct ui_cache_pixel_format_info g_ui_cache_pixel_format_infos[];

#ifdef __cplusplus
}
#endif

#endif
