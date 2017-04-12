#include <assert.h>
#include "ui_cache_pixel_format_i.h"

uint32_t ui_cache_pixel_format_to_stride(ui_cache_pixel_format_t format) {
    return g_ui_cache_pixel_format_infos[format].stride;
}

int8_t ui_cache_pixel_format_compressed(ui_cache_pixel_format_t format) {
    return g_ui_cache_pixel_format_infos[format].compressed;
}

uint8_t ui_cache_pixel_format_is_alpha_zero(ui_cache_pixel_format_t format, void const * data) {
    struct ui_cache_pixel_format_info * format_info = g_ui_cache_pixel_format_infos + format;

    return (*(((const char *)data) + format_info->alpha_start) & format_info->alpha_mask) == 0;
}

const char * ui_cache_pixel_format_name(ui_cache_pixel_format_t format) {
    return g_ui_cache_pixel_format_infos[format].name;
}

#if ! defined(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG)
#    define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0
#endif

#if ! defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
#    define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0
#endif

#if ! defined(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG)
#    define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0
#endif

#if ! defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
#    define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0
#endif

struct ui_cache_pixel_format_info
g_ui_cache_pixel_format_infos[ui_cache_pf_format_count] = {
    /*               stride  compressed, alpha_start, alpha_mask */
    { "unknown",     0,      0,          0          ,  0 }, 
    { "pal8",        1,      0,          0          ,  0 }, 
    { "pala8",       2,      0,          1          ,  0xFF }, 
    { "r5g6b5",      2,      0,          0          ,  0 }, 
    { "r4g4b4a4",    2,      0,          1          ,  0x0F }, 
    { "r5g5b5a1",    2,      0,          1          ,  0x01 }, 
    { "r8g8b8",      3,      0,          0          ,  0 }, 
    { "r8g8b8a8",    4,      0,          3          ,  0xFF }, 
    { "a8",          1,      0,          0          ,  0xFF }, 
    { "s8",          1,      0,          0          ,  0 }, 
    { "d16",         2,      0,          0          ,  0 }, 
    { "dxt1",        0,      1,          0          ,  0 }, 
    { "dxt3",        0,      1,          0          ,  0 }, 
    { "dxt5",        0,      1,          0          ,  0 }, 
    { "rgbpvrtc2",   2,      1,          0          ,  0 }, 
    { "rgbapvrtc2",  2,      1,          0          ,  0 }, 
    { "rgbpvrtc4",   4,      0,          0          ,  0 }, 
    { "rgbapvrtc4",  4,      1,          0          ,  0 }, 
};
    
