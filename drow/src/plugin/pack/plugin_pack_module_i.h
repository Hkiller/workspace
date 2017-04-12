#ifndef PLUGIN_PACK_MODULE_I_H
#define PLUGIN_PACK_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "plugin/pack/plugin_pack_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_pack_block * plugin_pack_block_t;
typedef struct plugin_pack_block_ref * plugin_pack_block_ref_t;
typedef struct plugin_pack_texture_part * plugin_pack_texture_part_t;
    
typedef TAILQ_HEAD(plugin_pack_language_list, plugin_pack_language) plugin_pack_language_list_t;
typedef TAILQ_HEAD(plugin_pack_texture_list, plugin_pack_texture) plugin_pack_texture_list_t;
typedef TAILQ_HEAD(plugin_pack_texture_part_list, plugin_pack_texture_part) plugin_pack_texture_part_list_t;
typedef TAILQ_HEAD(plugin_pack_block_list, plugin_pack_block) plugin_pack_block_list_t;    
typedef TAILQ_HEAD(plugin_pack_block_ref_list, plugin_pack_block_ref) plugin_pack_block_ref_list_t;

struct plugin_pack_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;
    ui_ed_mgr_t m_ed_mgr;
    uint8_t m_debug;
    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif 
