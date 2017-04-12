#include <assert.h>
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_page_eh_i.h"

plugin_ui_page_eh_t
plugin_ui_page_eh_create(
    plugin_ui_page_t page,
    const char * event, plugin_ui_page_eh_scope_t scope,
    plugin_ui_page_eh_fun_t fun, void * ctx)
{
    plugin_ui_env_t env = page->m_env;
    plugin_ui_page_eh_t eh;

    eh = TAILQ_FIRST(&env->m_free_page_ehs);
    if (eh) {
        TAILQ_REMOVE(&env->m_free_page_ehs, eh, m_next);
    }
    else {
        eh = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_page_eh) + env->m_backend->eh_capacity);
        if (eh == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_page_eh_create: alloc fail");
            return NULL;
        }
    }

    eh->m_page = page;
    eh->m_is_processing = 0;
    eh->m_is_free = 0;
    eh->m_is_active = 0;
    eh->m_event = event;
    eh->m_scope = scope;
    eh->m_fun = fun;
    eh->m_ctx = ctx;

    if (env->m_backend->eh_init(env->m_backend->ctx, eh) != 0) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_page_eh_create: backend init eh fail");
        mem_free(env->m_module->m_alloc, eh);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&page->m_ehs, eh, m_next);

    if (plugin_ui_page_eh_sync_active(eh) != 0) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_page_eh_create: first sync fail");
        plugin_ui_page_eh_free(eh);
        return NULL;
    }

    return eh;
}

void plugin_ui_page_eh_free(plugin_ui_page_eh_t eh) {
    plugin_ui_env_t env = eh->m_page->m_env;

    assert(!eh->m_is_free);
    if (eh->m_is_processing) {
        eh->m_is_free = 1;
        return;
    }

    if (eh->m_is_active) {
        eh->m_page->m_env->m_backend->eh_deactive(env->m_backend->ctx, eh);
        eh->m_is_active = 0;
    }

    env->m_backend->eh_fini(env->m_backend->ctx, eh);

    TAILQ_REMOVE(&eh->m_page->m_ehs, eh, m_next);

    eh->m_page = (void*)env;
    
    TAILQ_INSERT_TAIL(&env->m_free_page_ehs, eh, m_next);
}

void plugin_ui_page_eh_real_free(plugin_ui_page_eh_t eh) {
    plugin_ui_env_t env = (plugin_ui_env_t)eh->m_page;

    TAILQ_REMOVE(&env->m_free_page_ehs, eh, m_next);

    mem_free(env->m_module->m_alloc, eh);
}

const char * plugin_ui_page_eh_event(plugin_ui_page_eh_t eh) {
    return eh->m_event;
}

void * plugin_ui_page_eh_data(plugin_ui_page_eh_t eh) {
    return eh + 1;
}

void plugin_ui_page_eh_call(plugin_ui_page_eh_t eh, LPDRMETA data_meta, void const * data, size_t data_size) {
    eh->m_fun(eh->m_ctx, eh->m_page, data_meta, data, data_size);
}

int plugin_ui_page_eh_sync_active(plugin_ui_page_eh_t eh) {
    uint8_t need_active =
        (eh->m_scope == plugin_ui_page_eh_scope_all
         || (eh->m_scope == plugin_ui_page_eh_scope_visible && plugin_ui_page_visible(eh->m_page)))
        ? 1
        : 0;

    if (need_active != eh->m_is_active) {
        plugin_ui_env_t env = eh->m_page->m_env;

        if (need_active) {
            if (env->m_backend->eh_active(env->m_backend->ctx, eh) != 0) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_page_eh_sync_active: active fail!");
                return -1;
            }
        }
        else {
            eh->m_page->m_env->m_backend->eh_deactive(env->m_backend->ctx, eh);
        }
        
        eh->m_is_active = need_active;
    }

    return 0;
}
