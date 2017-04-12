#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_ui_move_algorithm_meta_i.h"
#include "plugin_ui_move_algorithm_i.h"

plugin_ui_move_algorithm_meta_t
plugin_ui_move_algorithm_meta_create(
    plugin_ui_module_t module,
    const char * type_name,
    void * ctx,
    size_t capacity,
    plugin_ui_move_alogrithm_init_fun_t init,
    plugin_ui_move_alogrithm_fini_fun_t fini,
    plugin_ui_move_alogrithm_calc_duration_fun_t calc_duration,
    plugin_ui_move_alogrithm_calc_pos_fun_t calc_pos,
    plugin_ui_move_algorithm_setup_fun_t setup)
{
    plugin_ui_move_algorithm_meta_t meta;

    meta = mem_alloc(module->m_alloc, sizeof(struct plugin_ui_move_algorithm_meta));
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_animation_type: alloc fail!");
        return NULL;
    }

    meta->m_module = module;
    meta->m_name = type_name;
    meta->m_ctx = ctx;
    meta->m_capacity = capacity;
    meta->m_init = init;
    meta->m_fini = fini;
    meta->m_calc_duration = calc_duration;
    meta->m_calc_pos = calc_pos;
    meta->m_setup = setup;
    TAILQ_INIT(&meta->m_algorithms);
    
    cpe_hash_entry_init(&meta->m_hh);
    if (cpe_hash_table_insert(&module->m_move_algorithm_metas, meta) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_move_algorithm_meta_create: meta name %s duplicate!", meta->m_name);
        mem_free(module->m_alloc, meta);
        return NULL;
    }

    if (capacity > module->m_move_algorithm_max_capacity) {
        module->m_move_algorithm_max_capacity = capacity;
    }
    
    return meta;
}

void plugin_ui_move_algorithm_meta_free(plugin_ui_move_algorithm_meta_t meta) {
    plugin_ui_module_t module = meta->m_module;

    cpe_hash_table_remove_by_ins(&module->m_move_algorithm_metas, meta);

    while(!TAILQ_EMPTY(&meta->m_algorithms)) {
        plugin_ui_move_algorithm_free(TAILQ_FIRST(&meta->m_algorithms));
    }

    mem_free(module->m_alloc, meta);
}

plugin_ui_move_algorithm_meta_t
plugin_ui_move_algorithm_meta_find(plugin_ui_module_t module, const char * name) {
    struct plugin_ui_move_algorithm_meta key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_move_algorithm_metas, &key);
}

void plugin_ui_move_algorithm_meta_free_all(const plugin_ui_module_t module) {
    struct cpe_hash_it move_algorithm_meta_it;
    plugin_ui_move_algorithm_meta_t move_algorithm_meta;

    cpe_hash_it_init(&move_algorithm_meta_it, &module->m_move_algorithm_metas);

    move_algorithm_meta = cpe_hash_it_next(&move_algorithm_meta_it);
    while (move_algorithm_meta) {
        plugin_ui_move_algorithm_meta_t next = cpe_hash_it_next(&move_algorithm_meta_it);
        plugin_ui_move_algorithm_meta_free(move_algorithm_meta);
        move_algorithm_meta = next;
    }
}

uint32_t plugin_ui_move_algorithm_meta_hash(const plugin_ui_move_algorithm_meta_t meta) {
    return cpe_hash_str(meta->m_name, strlen(meta->m_name));
}

int plugin_ui_move_algorithm_meta_eq(const plugin_ui_move_algorithm_meta_t l, const plugin_ui_move_algorithm_meta_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
