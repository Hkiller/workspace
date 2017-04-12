#include "cpe/pal/pal_strings.h"
#include "ui_sprite_world_res_i.h"
#include "ui_sprite_world_i.h"

ui_sprite_world_res_t ui_sprite_world_res_create(ui_sprite_world_t world, const char * name, size_t size) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_world_res_t world_res;
    size_t name_len = strlen(name) + 1;
    
    world_res = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_world_res) + size + name_len);
    if (world_res == NULL) {
        CPE_ERROR(repo->m_em, "create world res %s: alloc fail!", name);
        return NULL;
    }

    memcpy(((char*)(world_res + 1)) + size, name, name_len);

    bzero(world_res, sizeof(*world_res));

    world_res->m_world = world;
    world_res->m_name = ((char*)(world_res + 1)) + size;
    world_res->m_size = size;

    cpe_hash_entry_init(&world_res->m_hh_for_world);
    if (cpe_hash_table_insert_unique(&world->m_resources, world_res) != 0) {
        CPE_ERROR(repo->m_em, "create world res %s: name duplicate!", name);
        mem_free(repo->m_alloc, world_res);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&world->m_resources_by_order, world_res, m_next_for_world);

    return world_res;
}

void ui_sprite_world_res_free(ui_sprite_world_res_t world_res) {
    ui_sprite_world_t world = world_res->m_world;
    ui_sprite_repository_t repo = world->m_repo;

    if (world_res->m_free_fun) {
        world_res->m_free_fun(world_res, world_res->m_free_fun_ctx);
    }

    TAILQ_REMOVE(&world->m_resources_by_order, world_res, m_next_for_world);
    cpe_hash_table_remove_by_ins(&world->m_resources, world_res);

    mem_free(repo->m_alloc, world_res);
}

ui_sprite_world_res_t ui_sprite_world_res_find(ui_sprite_world_t world, const char * name) {
    struct ui_sprite_world_res key;
    key.m_name = name;
    return cpe_hash_table_find(&world->m_resources, &key);
}

void ui_sprite_world_res_set_free_fun(ui_sprite_world_res_t world_res, ui_sprite_world_res_free_fun_t fun, void * ctx) {
    world_res->m_free_fun = fun;
    world_res->m_free_fun_ctx = ctx;
}

ui_sprite_world_t ui_sprite_world_res_world(ui_sprite_world_res_t world_res) {
    return world_res->m_world;
}

void * ui_sprite_world_res_data(ui_sprite_world_res_t world_res) {
    return world_res + 1;
}

size_t ui_sprite_world_res_data_size(ui_sprite_world_res_t world_res) {
    return world_res->m_size;
}

ui_sprite_world_res_t ui_sprite_world_res_from_data(void * data) {
    return ((ui_sprite_world_res_t)data) - 1;
}

void ui_sprite_world_res_free_all(const ui_sprite_world_t world) {
    while(!TAILQ_EMPTY(&world->m_resources_by_order)) {
        ui_sprite_world_res_free(TAILQ_LAST(&world->m_resources_by_order, ui_sprite_world_res_list));
    }
}

uint32_t ui_sprite_world_res_hash(const ui_sprite_world_res_t world_res) {
    return cpe_hash_str(world_res->m_name, strlen(world_res->m_name));
}

int ui_sprite_world_res_eq(const ui_sprite_world_res_t l, const ui_sprite_world_res_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
