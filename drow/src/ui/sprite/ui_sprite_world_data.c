#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "ui_sprite_world_data_i.h"

static ui_sprite_world_data_t ui_sprite_world_create_data_i(ui_sprite_world_t world, LPDRMETA meta, size_t capacity) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_world_data_t world_data;
    
    world_data = mem_calloc(repo->m_alloc, sizeof(struct ui_sprite_world_data) + capacity);
    if (world_data == NULL) {
        CPE_ERROR(repo->m_em, "create world data %s: alloc fail!", dr_meta_name(meta));
        return NULL;
    }

    world_data->m_world = world;
    world_data->m_capacity = capacity;
    world_data->m_data.m_meta = meta;
    world_data->m_data.m_size = capacity;
    world_data->m_data.m_data = (void*)(world_data + 1);

    TAILQ_INSERT_TAIL(&world->m_datas, world_data, m_next);

    return world_data;
}

static ui_sprite_world_data_t ui_sprite_world_find_data_i(ui_sprite_world_t world, const char * name) {
    ui_sprite_world_data_t d;

    TAILQ_FOREACH(d, &world->m_datas, m_next) {
        if (strcmp(dr_meta_name(d->m_data.m_meta), name) == 0) return d;
    }

    return NULL;
}

void ui_sprite_world_data_free(ui_sprite_world_data_t world_data) {
    ui_sprite_world_t world = world_data->m_world;

    TAILQ_REMOVE(&world->m_datas, world_data, m_next);

    mem_free(world->m_repo->m_alloc, world_data);
}

dr_data_t ui_sprite_world_create_data(ui_sprite_world_t world, LPDRMETA meta, size_t capacity) {
    ui_sprite_world_data_t d = ui_sprite_world_find_data_i(world, dr_meta_name(meta));

    if (d && d->m_capacity < capacity) {
        ui_sprite_world_data_free(d);
        d = NULL;
    }

    if (d == NULL) d = ui_sprite_world_create_data_i(world, meta, capacity);
    
    return d ? &d->m_data : NULL;
}

dr_data_t ui_sprite_world_find_data(ui_sprite_world_t world, const char * name) {
    ui_sprite_world_data_t d;

    d = ui_sprite_world_find_data_i(world, name);

    return d ? &d->m_data : NULL;
}

int ui_sprite_world_set_data(ui_sprite_world_t world, dr_data_t data) {
    dr_data_t to_data = ui_sprite_world_create_data(world, data->m_meta, data->m_size) ;

    if (to_data == NULL) return -1;

    assert(to_data->m_size >= data->m_size);

    to_data->m_size = data->m_size;
    to_data->m_meta = data->m_meta;
    memcpy(to_data->m_data, data->m_data, data->m_size);

    return 0;
}

static dr_data_t plugin_ui_env_page_next(struct dr_data_it * it) {
    ui_sprite_world_data_t * data = (ui_sprite_world_data_t *)(it->m_data);
    ui_sprite_world_data_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next);
    return &r->m_data;
}

int ui_sprite_world_copy_datas(ui_sprite_world_t world, ui_sprite_world_t from_world) {
    int rv = 0;
    ui_sprite_world_data_t from_data;

    TAILQ_FOREACH(from_data, &from_world->m_datas, m_next) {
        if (ui_sprite_world_set_data(world, &from_data->m_data) != 0) {
            rv = -1;
        }
    }

    return rv;
}

void ui_sprite_world_datas(ui_sprite_world_t world, dr_data_it_t it) {
    *(ui_sprite_world_data_t *)(it->m_data) = TAILQ_FIRST(&world->m_datas);
    it->next = plugin_ui_env_page_next;
}

void ui_sprite_world_data_free_all(const ui_sprite_world_t world) {
    while(!TAILQ_EMPTY(&world->m_datas)) {
        ui_sprite_world_data_free(TAILQ_FIRST(&world->m_datas));
    }
}
