#include "plugin_app_env_delegate.h"

int plugin_app_env_ios_register_delegate(
    plugin_app_env_module_t app_env,
    void * ctx,
    plugin_app_env_ios_open_url_fun_t open_url)
{
    plugin_app_env_delegator_t delegator;

    delegator = mem_alloc(app_env->m_alloc, sizeof(struct plugin_app_env_delegator));
    if (delegator == NULL) {
        CPE_ERROR(app_env->m_em, "plugin_app_env_ios_register_delegate: alloc fail!");
        return -1;
    }

    delegator->m_backend = app_env->m_backend;
    delegator->m_ctx = ctx;
    delegator->m_open_url = open_url;
    
    TAILQ_INSERT_TAIL(&app_env->m_backend->m_delegators, delegator, m_next);
    
    return 0;
}

void plugin_app_env_ios_unregister_delegate(plugin_app_env_module_t app_env, void * ctx) {
    plugin_app_env_delegator_t delegator, next_delegator;

    for(delegator = TAILQ_FIRST(&app_env->m_backend->m_delegators); delegator; delegator = next_delegator) {
        next_delegator = TAILQ_NEXT(delegator, m_next);

        if (delegator->m_ctx == ctx) {
            TAILQ_REMOVE(&app_env->m_backend->m_delegators, delegator, m_next);
            mem_free(app_env->m_alloc, delegator);
        }
    }
}

uint8_t plugin_app_env_ios_dispatch_open_url(plugin_app_env_module_t app_env, void * application, void * url, void * sourceApplication, void * annotation) {
    plugin_app_env_delegator_t delegator;

    TAILQ_FOREACH(delegator, &app_env->m_backend->m_delegators, m_next) {
        if (delegator->m_open_url) {
            if (delegator->m_open_url(delegator->m_ctx, application, url, sourceApplication,  annotation)) {
                return 1;
            }
        }
    }
    
    return 0;
}

