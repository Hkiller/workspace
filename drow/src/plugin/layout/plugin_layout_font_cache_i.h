#ifndef PLUGIN_LAYOUT_FONT_CACHE_I_H
#define PLUGIN_LAYOUT_FONT_CACHE_I_H
#include "cpe/utils/utils_types.h"
#include "plugin/layout/plugin_layout_font_cache.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_font_cache {
    plugin_layout_module_t m_module;
    uint32_t m_width;
    uint32_t m_height;
    ui_cache_res_t m_texture;
    binpack_maxrects_ctx_t m_texture_alloc;
};

int plugin_layout_font_cache_register(plugin_layout_module_t module); 
void plugin_layout_font_cache_unregister(plugin_layout_module_t module);

#ifdef __cplusplus
}
#endif

#endif 
