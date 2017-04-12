#ifndef PLUGIN_SWF_MODULE_I_H
#define PLUGIN_SWF_MODULE_I_H
#include "gameswf/gameswf_types.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "plugin/swf/plugin_swf_module.h"

using namespace gameswf;
typedef struct plugin_swf_bitmap * plugin_swf_bitmap_t;
typedef struct plugin_swf_render_handler * plugin_swf_render_handler_t;
typedef struct plugin_swf_glyph_provider * plugin_swf_glyph_provider_t;

typedef TAILQ_HEAD(plugin_swf_bitmap_list, plugin_swf_bitmap) plugin_swf_bitmap_list_t;

struct plugin_swf_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;

    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;

    plugin_swf_render_handler_t m_render;
    gc_ptr<player> m_player;
    
    struct mem_buffer m_dump_buffer;
};

int plugin_swf_module_create_shaders(plugin_swf_module_t module);
void plugin_swf_module_free_shaders(plugin_swf_module_t module);

#endif
