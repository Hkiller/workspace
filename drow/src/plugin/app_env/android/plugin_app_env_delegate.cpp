#include "plugin_app_env_delegate.h"

extern "C"
int plugin_app_env_android_register_delegate(
    plugin_app_env_module_t app_env,
    void * ctx,
    plugin_app_env_android_on_new_intent_fun_t new_intent,
    plugin_app_env_android_on_activity_result_fun_t on_activity_result)
{
    plugin_app_env_delegator_t delegator;

    delegator = (plugin_app_env_delegator_t)mem_alloc(app_env->m_alloc, sizeof(struct plugin_app_env_delegator));
    if (delegator == NULL) {
        CPE_ERROR(app_env->m_em, "plugin_app_env_android_register_delegate: alloc fail!");
        return -1;
    }

    delegator->m_backend = app_env->m_backend;
    delegator->m_ctx = ctx;
    delegator->m_new_intent = new_intent;
    delegator->m_on_activity_result = on_activity_result;
    
    TAILQ_INSERT_TAIL(&app_env->m_backend->m_delegators, delegator, m_next);
    
    return 0;
}

extern "C"
void plugin_app_env_android_unregister_delegate(plugin_app_env_module_t app_env, void * ctx) {
    plugin_app_env_delegator_t delegator, next_delegator;

    for(delegator = TAILQ_FIRST(&app_env->m_backend->m_delegators); delegator; delegator = next_delegator) {
        next_delegator = TAILQ_NEXT(delegator, m_next);

        if (delegator->m_ctx == ctx) {
            TAILQ_REMOVE(&app_env->m_backend->m_delegators, delegator, m_next);
            mem_free(app_env->m_alloc, delegator);
        }
    }
}

extern "C"
void plugin_app_env_android_dispatch_new_intent(plugin_app_env_module_t app_env, void * intent) {
    plugin_app_env_delegator_t delegator;

    TAILQ_FOREACH(delegator, &app_env->m_backend->m_delegators, m_next) {
        if (delegator->m_new_intent) {
            delegator->m_new_intent(delegator->m_ctx, intent);
        }
    }
}

extern "C"
void plugin_app_env_android_dispatch_activity_result(plugin_app_env_module_t app_env, void * intent, int requestCode, int resultCode) {
    plugin_app_env_delegator_t delegator;

    TAILQ_FOREACH(delegator, &app_env->m_backend->m_delegators, m_next) {
        if (delegator->m_on_activity_result) {
            delegator->m_on_activity_result(delegator->m_ctx, intent, requestCode, resultCode);
        }
    }
}

