#include <assert.h>
#include "plugin_app_env_backend.h"

int plugin_app_env_backend_init(plugin_app_env_module_t module) {
    plugin_app_env_backend_t backend;

    backend = mem_alloc(module->m_alloc, sizeof(struct plugin_app_env_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "plugin_app_env_backend_init: alloc fail!");
        return -1;
    }

    backend->m_module = module;
    backend->m_window = NULL;
    backend->m_application = NULL;
    TAILQ_INIT(&backend->m_delegators);
    
    module->m_backend = backend;
    return 0;
}

void plugin_app_env_backend_fini(plugin_app_env_module_t module) {
    assert(module->m_backend);

    assert(TAILQ_EMPTY(&module->m_backend->m_delegators));

    mem_free(module->m_alloc, module->m_backend);
    module->m_backend = NULL;
}


void plugin_app_env_ios_set_window(plugin_app_env_module_t app_env, void * window) {
    app_env->m_backend->m_window = window;
}

void * plugin_app_env_ios_window(plugin_app_env_module_t app_env) {
    return app_env->m_backend->m_window;
}

void plugin_app_env_ios_set_application(plugin_app_env_module_t app_env, void * application) {
    app_env->m_backend->m_application = application;
}

void * plugin_app_env_ios_application(plugin_app_env_module_t app_env) {
    return app_env->m_backend->m_application;
}
