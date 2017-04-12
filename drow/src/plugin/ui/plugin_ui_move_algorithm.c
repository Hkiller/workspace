#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "plugin_ui_move_algorithm_i.h"
#include "plugin_ui_move_algorithm_meta_i.h"
#include "plugin_ui_env_i.h"

plugin_ui_move_algorithm_t
plugin_ui_move_algorithm_create(plugin_ui_env_t env, plugin_ui_move_algorithm_meta_t meta) {
    plugin_ui_module_t module = env->m_module;
    plugin_ui_move_algorithm_t move_algorithm;

    move_algorithm = TAILQ_FIRST(&env->m_free_move_algorithms);
    if (move_algorithm) {
        TAILQ_REMOVE(&env->m_free_move_algorithms, move_algorithm, m_next_for_env);
    }
    else {
        move_algorithm = mem_alloc(module->m_alloc, sizeof(struct plugin_ui_move_algorithm) + module->m_move_algorithm_max_capacity);
        if (move_algorithm == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_move_algorithm: create: alloc fail!");
            return NULL;
        }
    }

    move_algorithm->m_env = env;
    move_algorithm->m_meta = meta;
    move_algorithm->m_id = env->m_max_move_algorithm_id + 1;
    move_algorithm->m_user_ctx = NULL;
    move_algorithm->m_user_on_fini = NULL;

    bzero(move_algorithm + 1, module->m_move_algorithm_max_capacity);
    
    if (meta->m_init && meta->m_init(move_algorithm, meta->m_ctx) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_move_algorithm: type %s init fail!", meta->m_name);
        TAILQ_INSERT_TAIL(&env->m_free_move_algorithms, move_algorithm, m_next_for_env);
        return NULL;
    }
    
    cpe_hash_entry_init(&move_algorithm->m_hh);
    if (cpe_hash_table_insert(&env->m_move_algorithms, move_algorithm) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_move_algorithm: id %d duplicate!", move_algorithm->m_id);
        TAILQ_INSERT_TAIL(&env->m_free_move_algorithms, move_algorithm, m_next_for_env);
        return NULL;
    }
    TAILQ_INSERT_TAIL(&meta->m_algorithms, move_algorithm, m_next_for_meta);
    
    env->m_max_move_algorithm_id++;
    return move_algorithm;
}

plugin_ui_move_algorithm_t
plugin_ui_move_algorithm_create_by_type_name(plugin_ui_env_t env, const char * type_name) {
    plugin_ui_move_algorithm_meta_t meta;

    meta = plugin_ui_move_algorithm_meta_find(env->m_module, type_name);
    if (meta == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_move_algorithm: create: meta %s not exist!", type_name);
        return NULL;
    }

    return plugin_ui_move_algorithm_create(env, meta);
}

void plugin_ui_move_algorithm_free(plugin_ui_move_algorithm_t move_algorithm) {
    if (move_algorithm->m_user_on_fini) {
        move_algorithm->m_user_on_fini(move_algorithm, move_algorithm->m_user_ctx);
        move_algorithm->m_user_on_fini = NULL;
    }
    
    if (move_algorithm->m_meta->m_fini) {
        move_algorithm->m_meta->m_fini(move_algorithm, move_algorithm->m_meta->m_ctx);
    }

    TAILQ_REMOVE(&move_algorithm->m_meta->m_algorithms, move_algorithm, m_next_for_meta);
    cpe_hash_table_remove_by_ins(&move_algorithm->m_env->m_move_algorithms, move_algorithm);

    TAILQ_INSERT_TAIL(&move_algorithm->m_env->m_free_move_algorithms, move_algorithm, m_next_for_env);
}
    
void plugin_ui_move_algorithm_real_free(plugin_ui_move_algorithm_t move_algorithm) {
    TAILQ_REMOVE(&move_algorithm->m_env->m_free_move_algorithms, move_algorithm, m_next_for_env);
    mem_free(move_algorithm->m_env->m_module->m_alloc, move_algorithm);
}

plugin_ui_env_t plugin_ui_move_algorithm_env(plugin_ui_move_algorithm_t move_algorithm) {
    return move_algorithm->m_env;
}

uint32_t plugin_ui_move_algorithm_id(plugin_ui_move_algorithm_t move_algorithm) {
    return move_algorithm->m_id;
}

void * plugin_ui_move_algorithm_data(plugin_ui_move_algorithm_t move_algorithm) {
    return move_algorithm + 1;
}

plugin_ui_move_algorithm_t plugin_ui_move_algorithm_from_data(void * data) {
    return ((plugin_ui_move_algorithm_t)data) - 1;
}

plugin_ui_move_algorithm_t
plugin_ui_move_algorithm_find(plugin_ui_env_t env, uint32_t move_algorithm_id) {
    struct plugin_ui_move_algorithm key;
    key.m_id = move_algorithm_id;
    return cpe_hash_table_find(&env->m_move_algorithms, &key);
}

int plugin_ui_move_algorithm_setup(plugin_ui_move_algorithm_t move_algorithm, char * arg_buf_will_change) {
    plugin_ui_env_t env = move_algorithm->m_env;
    
    if (move_algorithm->m_meta->m_setup) {
        if (move_algorithm->m_meta->m_setup(move_algorithm, move_algorithm->m_meta->m_ctx, arg_buf_will_change) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "move_algorithm %d(%s): setup from %s fail",
                move_algorithm->m_id, move_algorithm->m_meta->m_name, arg_buf_will_change);
            return -1;
        }
    }

    return 0;
}

uint32_t plugin_ui_move_algorithm_hash(const plugin_ui_move_algorithm_t meta) {
    return meta->m_id;
}

int plugin_ui_move_algorithm_eq(const plugin_ui_move_algorithm_t l, const plugin_ui_move_algorithm_t r) {
    return l->m_id == r->m_id ? 1 : 0;
}
