#ifndef PLUGIN_LAYOUT_FONT_CACHE_H
#define PLUGIN_LAYOUT_FONT_CACHE_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_font_cache_t plugin_layout_font_cache_get(plugin_layout_module_t module);
int plugin_layout_font_cache_set_size(plugin_layout_font_cache_t cache, uint32_t width, uint32_t height);

int plugin_layout_font_cache_insert(
    plugin_layout_font_cache_t cache, ui_rect_t r_rect, uint32_t width, uint32_t height,
    void const * data, size_t data_size);
int plugin_layout_font_cache_clear(plugin_layout_font_cache_t cache);

#ifdef __cplusplus
}
#endif

#endif
