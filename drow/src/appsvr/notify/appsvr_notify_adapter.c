#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "appsvr_notify_adapter_i.h"
#include "appsvr_notify_activate_i.h"
#include "appsvr_notify_tag_adapter_i.h"

struct appsvr_notify_adapter_add_tag_ctx {
    appsvr_notify_adapter_t m_adapter;
    int m_rv;
};

static void appsvr_notify_adapter_add_to_tag(void * i_ctx, const char * value) {
    struct appsvr_notify_adapter_add_tag_ctx * ctx = i_ctx;
    appsvr_notify_tag_t tag;

    tag = appsvr_notify_tag_check_create(ctx->m_adapter->m_module, value);
    if (tag == NULL) {
        ctx->m_rv = -1;
        return;
    }

    if (appsvr_notify_tag_adapter_create(tag, ctx->m_adapter) == NULL) {
        ctx->m_rv = -1;
        return;
    }
}

appsvr_notify_adapter_t
appsvr_notify_adapter_create(
    appsvr_notify_module_t module, const char * name,
    void * ctx,
    appsvr_notify_install_fun_t install_fun,
    appsvr_notify_update_fun_t update_fun,
    appsvr_notify_uninstall_fun_t uninstall_fun,
    const char * tags)
{
    appsvr_notify_adapter_t adapter;
    struct appsvr_notify_adapter_add_tag_ctx add_tag_ctx;
    
    if (tags == NULL || tags[0] == 0) tags = "default";

    adapter = mem_calloc(module->m_alloc, sizeof(struct appsvr_notify_adapter));
    if (adapter == NULL) {
        CPE_ERROR(module->m_em, "ad: create adapter %s: alloc fail!", name);
        return NULL;
    }

    adapter->m_module = module;
    cpe_str_dup(adapter->m_name, sizeof(adapter->m_name), name);
    adapter->m_ctx = ctx;
    adapter->m_install_fun = install_fun;
    adapter->m_update_fun = update_fun;
    adapter->m_uninstall_fun = uninstall_fun;
    TAILQ_INIT(&adapter->m_tags);
    TAILQ_INIT(&adapter->m_activates);
    
    TAILQ_INSERT_TAIL(&module->m_adapters, adapter, m_next);

    /*绑定所有tag */
    add_tag_ctx.m_adapter = adapter;
    add_tag_ctx.m_rv = 0;
    cpe_str_list_for_each(tags, ':', appsvr_notify_adapter_add_to_tag, &add_tag_ctx);
    if (add_tag_ctx.m_rv != 0) {
        appsvr_notify_adapter_free(adapter);
        return NULL;
    }
    
    return adapter;
}

void appsvr_notify_adapter_free(appsvr_notify_adapter_t adapter) {
    appsvr_notify_module_t module;

    module = adapter->m_module;

    while(!TAILQ_EMPTY(&adapter->m_activates)) {
        appsvr_notify_activate_free(TAILQ_FIRST(&adapter->m_activates));
    }

    while(!TAILQ_EMPTY(&adapter->m_tags)) {
        appsvr_notify_tag_adapter_free(TAILQ_FIRST(&adapter->m_tags));
    }

    TAILQ_REMOVE(&module->m_adapters, adapter, m_next);

    mem_free(module->m_alloc, adapter);
}

