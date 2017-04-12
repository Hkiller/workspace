#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_sprite_component_meta_i.h"
#include "ui_sprite_component_i.h"

ui_sprite_component_meta_t
ui_sprite_component_meta_create(
    ui_sprite_repository_t repo, const char * name, uint16_t data_size)
{
    ui_sprite_component_meta_t meta;
    size_t name_len = strlen(name) + 1;

    meta = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_component_meta) + name_len);
    if (meta == NULL) {
        CPE_ERROR(repo->m_em, "create component meta %s: alloc fail !", name);
        return NULL;
    }

    bzero(meta, sizeof(struct ui_sprite_component_meta));

	meta->m_repo = repo;
    TAILQ_INIT(&meta->m_components);
    meta->m_name = (void*)(meta + 1);

    memcpy(meta + 1, name, name_len); 
    meta->m_size = data_size;

    cpe_hash_entry_init(&meta->m_hh_for_repo);
    if (cpe_hash_table_insert_unique(&repo->m_component_metas, meta) != 0) {
        CPE_ERROR(repo->m_em, "create component meta %s: name duplicate!", name);
        mem_free(repo->m_alloc, meta);
        return NULL;
    }

    return meta;
}

void ui_sprite_component_meta_free(ui_sprite_component_meta_t meta) {
    ui_sprite_repository_t repo = meta->m_repo;

    while(!TAILQ_EMPTY(&meta->m_components)) {
        ui_sprite_component_free(TAILQ_FIRST(&meta->m_components));
    }

    cpe_hash_table_remove_by_ins(&repo->m_component_metas, meta);

    mem_free(repo->m_alloc, meta);
}

ui_sprite_component_meta_t
ui_sprite_component_meta_find(ui_sprite_repository_t repo, const char * name) {
    struct ui_sprite_component_meta key;

    key.m_name = name;
    return cpe_hash_table_find(&repo->m_component_metas, &key);
}

void ui_sprite_component_meta_set_data_meta(
    ui_sprite_component_meta_t meta, LPDRMETA data_meta, uint16_t start_pos, uint16_t data_size)
{
    assert(start_pos + data_size <= meta->m_size);
    meta->m_data_meta = data_meta;
    meta->m_data_start = start_pos;
    meta->m_data_size = data_size;
}

void ui_sprite_component_meta_set_enter_fun(ui_sprite_component_meta_t meta, ui_sprite_component_enter_fun_t fun, void * ctx) {
    meta->m_enter_fun_ctx = ctx;
    meta->m_enter_fun = fun;
}

void ui_sprite_component_meta_set_exit_fun(ui_sprite_component_meta_t meta, ui_sprite_component_exit_fun_t fun, void * ctx) {
    meta->m_exit_fun_ctx = ctx;
    meta->m_exit_fun = fun;
}

void ui_sprite_component_meta_set_init_fun(ui_sprite_component_meta_t meta, ui_sprite_component_init_fun_t fun, void * ctx) {
    meta->m_init_fun_ctx = ctx;
    meta->m_init_fun = fun;
}

void ui_sprite_component_meta_set_copy_fun(ui_sprite_component_meta_t meta, ui_sprite_component_copy_fun_t fun, void * ctx) {
    meta->m_copy_fun_ctx = ctx;
    meta->m_copy_fun = fun;
}

void ui_sprite_component_meta_set_free_fun(ui_sprite_component_meta_t meta, ui_sprite_component_free_fun_t fun, void * ctx) {
    meta->m_free_fun_ctx = ctx;
    meta->m_free_fun = fun;
}

void ui_sprite_component_meta_set_update_fun(ui_sprite_component_meta_t meta, ui_sprite_component_update_fun_t fun, void * ctx) {
    meta->m_update_fun_ctx = ctx;
    meta->m_update_fun = fun;
}

void ui_sprite_component_meta_free_all(ui_sprite_repository_t repo) {
    struct cpe_hash_it meta_it;
    ui_sprite_component_meta_t meta;

    cpe_hash_it_init(&meta_it, &repo->m_component_metas);

    meta = cpe_hash_it_next(&meta_it);
    while (meta) {
        ui_sprite_component_meta_t next = cpe_hash_it_next(&meta_it);
        ui_sprite_component_meta_free(meta);
        meta = next;
    }
}

uint32_t ui_sprite_component_meta_hash(const ui_sprite_component_meta_t meta) {
    return cpe_hash_str(meta->m_name, strlen(meta->m_name));
}

int ui_sprite_component_meta_eq(const ui_sprite_component_meta_t l, const ui_sprite_component_meta_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}