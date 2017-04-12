#ifndef PLUGIN_BARRAGE_MODULE_I_H
#define PLUGIN_BARRAGE_MODULE_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "plugin/barrage/plugin_barrage_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_barrage_data * plugin_barrage_data_t;
typedef struct plugin_barrage_op * plugin_barrage_op_t;
typedef struct plugin_barrage_trigger_op * plugin_barrage_trigger_op_t;
typedef struct plugin_barrage_bullet_proto * plugin_barrage_bullet_proto_t;

typedef TAILQ_HEAD(plugin_barrage_data_emitter_list, plugin_barrage_data_emitter) plugin_barrage_data_emitter_list_t;
typedef TAILQ_HEAD(plugin_barrage_op_list, plugin_barrage_op) plugin_barrage_op_list_t;
typedef TAILQ_HEAD(plugin_barrage_group_list, plugin_barrage_group) plugin_barrage_group_list_t;
typedef TAILQ_HEAD(plugin_barrage_bullet_list, plugin_barrage_bullet) * plugin_barrage_bullet_list_t;
typedef TAILQ_HEAD(plugin_barrage_trigger_op_list, plugin_barrage_trigger_op) * plugin_barrage_trigger_op_list_t;
typedef TAILQ_HEAD(plugin_barrage_data_bullet_list, plugin_barrage_data_bullet) plugin_barrage_data_bullet_list_t;
typedef TAILQ_HEAD(plugin_barrage_data_bullets_use_list, plugin_barrage_data_bullets_use) plugin_barrage_data_bullets_use_list_t;
typedef TAILQ_HEAD(plugin_barrage_barrage_list, plugin_barrage_barrage) plugin_barrage_barrage_list_t;
typedef TAILQ_HEAD(plugin_barrage_emitter_list, plugin_barrage_emitter) plugin_barrage_emitter_list_t;

struct plugin_barrage_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;
    uint8_t m_debug;

    LPDRMETA m_meta_barrage_info;
    LPDRMETA m_meta_emitter_info;
    LPDRMETA m_meta_emitter_trigger_info;
    LPDRMETA m_meta_bullet_trigger_info;
    LPDRMETA m_meta_emitter;
    LPDRMETA m_meta_bullet;

    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif 
