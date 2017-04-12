#ifndef PLUGIN_APP_ENV_REQUEST_I_H
#define PLUGIN_APP_ENV_REQUEST_I_H
#include "plugin/app_env/plugin_app_env_request.h"
#include "plugin_app_env_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_app_env_request {
    plugin_app_env_module_t m_module;
    cpe_hash_entry m_hh;
    TAILQ_ENTRY(plugin_app_env_request) m_next_for_processing;
    
    uint32_t m_id;
    LPDRMETA m_request_meta;
    void * m_receiver_ctx;
    plugin_app_env_request_receiver_fun_t m_receiver_fun;
    void (*m_receiver_ctx_free)(void *);

    uint8_t m_is_runing;
    int m_rv;
    struct dr_data m_result;
};

plugin_app_env_request_t plugin_app_env_request_create(
    plugin_app_env_module_t module, LPDRMETA request_meta,
    void * receiver_ctx, plugin_app_env_request_receiver_fun_t receiver_fun, void (*receiver_ctx_free)(void *));
void plugin_app_env_request_free(plugin_app_env_request_t request);

int plugin_app_env_notify_request_result(
    plugin_app_env_module_t module, uint32_t id,
    int rv, LPDRMETA meta, void const * data, size_t data_size);
    
void plugin_app_env_request_free_all(plugin_app_env_module_t module);

uint32_t plugin_app_env_request_hash(plugin_app_env_request_t request);
int plugin_app_env_request_eq(plugin_app_env_request_t l, plugin_app_env_request_t r);

int plugin_app_env_request_init(plugin_app_env_module_t module);
void plugin_app_env_request_fini(plugin_app_env_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
