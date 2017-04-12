#ifndef PLUGIN_APP_ENV_MODULE_I_H
#define PLUGIN_APP_ENV_MODULE_I_H
#include "cpe/utils/hash.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_app_env_request_list, plugin_app_env_request) plugin_app_env_request_list_t; 
typedef struct plugin_app_env_backend * plugin_app_env_backend_t;
    
struct plugin_app_env_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    plugin_app_env_backend_t m_backend;

    struct cpe_hash_table m_executors;
    struct cpe_hash_table m_monitors;

    uint32_t m_max_request_id;
    struct cpe_hash_table m_requests;
    plugin_app_env_request_list_t m_processing_requests;

    struct mem_buffer m_dump_buffer;
    
    uint8_t m_suspend;
};

int plugin_app_env_backend_init(plugin_app_env_module_t module);
void plugin_app_env_backend_fini(plugin_app_env_module_t module);

#ifdef __cplusplus
}
#endif
    
#endif
