#include "ui_sprite_tri_action_meta_i.h"

ui_sprite_tri_action_meta_t
ui_sprite_tri_action_meta_create(
    ui_sprite_tri_module_t module, const char * type_name, size_t data_capacity,
    void * ctx,
    ui_sprite_tri_action_init_fun_t init,
    ui_sprite_tri_action_fini_fun_t fini,
    ui_sprite_tri_action_copy_fun_t copy,
    ui_sprite_tri_action_exec_fun_t exec)
{
    ui_sprite_tri_action_meta_t meta;
    size_t name_len = strlen(type_name) + 1;

    if (module->m_action_count != 0 || !TAILQ_EMPTY(&module->m_free_actions)) {
        CPE_ERROR(module->m_em, "ui_sprite_tri_action_meta_create: already have action!");
        return NULL;
    }
    
    meta = mem_calloc(module->m_alloc, sizeof(struct ui_sprite_tri_action_meta) + name_len);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tri_action_meta_create: alloc fail!");
        return NULL;
    }

    memcpy(meta + 1, type_name, name_len);
    
    meta->m_module = module;
    meta->m_name = (void*)(meta + 1);
    meta->m_data_capacity = data_capacity;
    meta->m_ctx = ctx;
    meta->m_init = init;
    meta->m_fini = fini;
    meta->m_copy = copy;
    meta->m_exec = exec;

    cpe_hash_entry_init(&meta->m_hh);
    if (cpe_hash_table_insert_unique(&module->m_action_metas, meta) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_tri_action_meta_create: meta name %s duplicate!", meta->m_name);
        mem_free(module->m_alloc, meta);
        return NULL;
    }

    if (meta->m_data_capacity > module->m_action_max_data_capacity) {
        module->m_action_max_data_capacity = meta->m_data_capacity;
    }
    
    return meta;
}

void ui_sprite_tri_action_meta_free(ui_sprite_tri_action_meta_t meta) {
    ui_sprite_tri_module_t module = meta->m_module;
    
    cpe_hash_table_remove_by_ins(&module->m_action_metas, meta);

    mem_free(module->m_alloc, meta);
}

ui_sprite_tri_action_meta_t
ui_sprite_tri_action_meta_find(ui_sprite_tri_module_t module, const char * name) {
    struct ui_sprite_tri_action_meta key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_action_metas, &key);
}

void ui_sprite_tri_action_meta_free_all(const ui_sprite_tri_module_t module) {
    struct cpe_hash_it action_meta_it;
    ui_sprite_tri_action_meta_t action_meta;

    cpe_hash_it_init(&action_meta_it, &module->m_action_metas);

    action_meta = cpe_hash_it_next(&action_meta_it);
    while (action_meta) {
        ui_sprite_tri_action_meta_t next = cpe_hash_it_next(&action_meta_it);
        ui_sprite_tri_action_meta_free(action_meta);
        action_meta = next;
    }
}

uint32_t ui_sprite_tri_action_meta_hash(const ui_sprite_tri_action_meta_t meta) {
    return cpe_hash_str(meta->m_name, strlen(meta->m_name));
}

int ui_sprite_tri_action_meta_eq(const ui_sprite_tri_action_meta_t l, const ui_sprite_tri_action_meta_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
