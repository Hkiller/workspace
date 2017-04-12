#ifndef PLUGIN_BASICANIM_MODULE_I_H
#define PLUGIN_BASICANIM_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "plugin/basicanim/plugin_basicanim_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_basicanim_actor_layer * plugin_basicanim_actor_layer_t;

typedef TAILQ_HEAD(plugin_basicanim_actor_layer_list, plugin_basicanim_actor_layer) plugin_basicanim_actor_layer_list_t;

struct plugin_basicanim_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    ui_data_mgr_t m_data_mgr;
    ui_runtime_module_t m_runtime;
};

#ifdef __cplusplus
}
#endif

#endif 
