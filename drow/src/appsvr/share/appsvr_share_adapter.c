#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "appsvr_share_adapter_i.h"
#include "appsvr_share_request_i.h"

appsvr_share_adapter_t
appsvr_share_adapter_create(
    appsvr_share_module_t module, const char * name,
    void * ctx, appsvr_share_commit_fun_t commit_fun)
{
    appsvr_share_adapter_t adapter;
    
    adapter = mem_calloc(module->m_alloc, sizeof(struct appsvr_share_adapter));
    if (adapter == NULL) {
        CPE_ERROR(module->m_em, "ad: create adapter %s: alloc fail!", name);
        return NULL;
    }

    adapter->m_module = module;
    cpe_str_dup(adapter->m_name, sizeof(adapter->m_name), name);
    adapter->m_ctx = ctx;
    adapter->m_commit_fun = commit_fun;
    TAILQ_INIT(&adapter->m_requests);
    
    TAILQ_INSERT_TAIL(&module->m_adapters, adapter, m_next);

    return adapter;
}

void appsvr_share_adapter_free(appsvr_share_adapter_t adapter) {
    appsvr_share_module_t module;

    module = adapter->m_module;

    while(!TAILQ_EMPTY(&adapter->m_requests)) {
        appsvr_share_request_t request = TAILQ_FIRST(&adapter->m_requests);

        assert(request->m_adapter == adapter);
        
    }

    TAILQ_REMOVE(&module->m_adapters, adapter, m_next);

    mem_free(module->m_alloc, adapter);
}

static appsvr_share_adapter_t appsvr_share_module_adapter_next(struct appsvr_share_adapter_it * it) {
    appsvr_share_adapter_t * data = (appsvr_share_adapter_t *)(it->m_data);
    appsvr_share_adapter_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next);
    return r;
}

void appsvr_share_module_adapters(appsvr_share_module_t module, appsvr_share_adapter_it_t it) {
    *(appsvr_share_adapter_t *)(it->m_data) = TAILQ_FIRST(&module->m_adapters);
    it->next = appsvr_share_module_adapter_next;
}

appsvr_share_adapter_t appsvr_share_adapter_first(appsvr_share_module_t module) {
    return TAILQ_FIRST(&module->m_adapters);
}

