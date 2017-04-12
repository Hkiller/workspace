#include <assert.h>
#include "stdio.h"
#include "cpe/pal/pal_strings.h"
#include "gd/app/app_context.h"
#include "plugin_tiledmap_env_i.h"
#include "plugin_tiledmap_layer_i.h"
#include "plugin_tiledmap_tile_i.h"

plugin_tiledmap_env_t
plugin_tiledmap_env_create(plugin_tiledmap_module_t module) {
    plugin_tiledmap_env_t env;

    env = mem_alloc(module->m_alloc, sizeof(struct plugin_tiledmap_env));
    if (env == NULL) {
        CPE_ERROR(module->m_em, "create tiledmap env: alloc fail!");
        return NULL;
    }

    bzero(env, sizeof(*env));

    env->m_module = module;
    env->m_extern_obj_create_ctx = NULL;
    env->m_extern_obj_create_fun = NULL;

    if (cpe_hash_table_init(
            &env->m_layers,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_tiledmap_hash,
            (cpe_hash_eq_t) plugin_tiledmap_eq,
            CPE_HASH_OBJ2ENTRY(plugin_tiledmap_layer, m_hh_for_env),
            -1) != 0)
    {
        mem_free(module->m_alloc, env);
        return NULL;
    }

    TAILQ_INIT(&env->m_free_tiles);

    return env;
}

void plugin_tiledmap_env_free(plugin_tiledmap_env_t env) {
    plugin_tiledmap_layer_free_all(env);

    cpe_hash_table_fini(&env->m_layers);
    
    while(!TAILQ_EMPTY(&env->m_free_tiles)) {
        plugin_tiledmap_tile_real_free(TAILQ_FIRST(&env->m_free_tiles));
    }

    mem_free(env->m_module->m_alloc, env);
}

void plugin_tiledmap_env_set_extern_obj_factory(
    plugin_tiledmap_env_t env, void * create_ctx, plugin_tiledmap_env_extern_obj_create_fun_t create_fun)
{
    env->m_extern_obj_create_fun = create_fun;
    env->m_extern_obj_create_ctx = create_ctx;
}
