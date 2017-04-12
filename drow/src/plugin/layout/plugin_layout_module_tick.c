#include "cpe/utils/binpack.h"
#include "plugin_layout_module_i.h"
#include "plugin_layout_font_cache_i.h"
#include "plugin_layout_font_element_i.h"

ptr_int_t plugin_layout_module_tick(void * ctx, ptr_int_t arg, float delta_s) {
    plugin_layout_module_t module = ctx;
    float cache_occupancy = binpack_maxrects_ctx_occupancy(module->m_font_cache->m_texture_alloc);
    
    if (cache_occupancy > 0.9f) {
        CPE_INFO(module->m_em, "plugin_layout_module: cache occupancy %f, auto reset!", cache_occupancy);
        plugin_layout_font_element_clear_not_used(module);
        plugin_layout_font_cache_clear(module->m_font_cache);
    }
    
    return 0;
}
