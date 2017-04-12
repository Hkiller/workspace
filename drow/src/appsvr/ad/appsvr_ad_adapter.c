#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "appsvr_ad_adapter_i.h"
#include "appsvr_ad_action_i.h"

appsvr_ad_adapter_t
appsvr_ad_adapter_create(
    appsvr_ad_module_t module, const char * name,
    void * ctx,
    appsvr_ad_start_fun_t req_start_fun,
    appsvr_ad_cancel_fun_t req_cancel_fun)
{
    appsvr_ad_adapter_t adapter;

    adapter = mem_calloc(module->m_alloc, sizeof(struct appsvr_ad_adapter));
    if (adapter == NULL) {
        CPE_ERROR(module->m_em, "ad: create adapter %s: alloc fail!", name);
        return NULL;
    }

    adapter->m_module = module;
    cpe_str_dup(adapter->m_name, sizeof(adapter->m_name), name);
    adapter->m_ctx = ctx;
    adapter->m_req_start_fun = req_start_fun;
    adapter->m_req_cancel_fun = req_cancel_fun;
    TAILQ_INIT(&adapter->m_actions);
    
    module->m_adapter_count++;
    TAILQ_INSERT_TAIL(&module->m_adapters, adapter, m_next);
        
    return adapter;
}

void appsvr_ad_adapter_free(appsvr_ad_adapter_t adapter) {
    appsvr_ad_module_t module;

    module = adapter->m_module;

    while(!TAILQ_EMPTY(&adapter->m_actions)) {
        appsvr_ad_action_free(TAILQ_FIRST(&adapter->m_actions));
    }

    module->m_adapter_count--;
    TAILQ_REMOVE(&module->m_adapters, adapter, m_next);

    mem_free(module->m_alloc, adapter);
}

