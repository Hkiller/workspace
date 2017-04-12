#ifndef PLUGIN_MASK_MODULE_I_H
#define PLUGIN_MASK_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/mask/plugin_mask_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_mask_data_list, plugin_mask_data) plugin_mask_data_list_t;
typedef TAILQ_HEAD(plugin_mask_data_bloci_list, plugin_mask_data_block) plugin_mask_data_block_list_t;

struct plugin_mask_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_data_mgr_t m_data_mgr;
    ui_runtime_module_t m_runtime;
    error_monitor_t m_em;

    plugin_mask_data_block_list_t m_free_data_blocks;

    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
