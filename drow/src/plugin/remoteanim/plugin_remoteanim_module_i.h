#ifndef PLUGIN_REMOTEANIM_MODULE_I_H
#define PLUGIN_REMOTEANIM_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "gd/app/app_context.h"
#include "plugin/remoteanim/plugin_remoteanim_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_remoteanim_block_list, plugin_remoteanim_block) plugin_remoteanim_block_list_t;
typedef TAILQ_HEAD(plugin_remoteanim_obj_list, plugin_remoteanim_obj) plugin_remoteanim_obj_list_t;
    
struct plugin_remoteanim_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    net_trans_manage_t m_trans_mgr;
    ui_runtime_module_t m_runtime;

    struct cpe_hash_table m_groups;
    struct cpe_hash_table m_blocks;
    plugin_remoteanim_block_list_t m_free_blocks;
};

#ifdef __cplusplus
}
#endif

#endif 
