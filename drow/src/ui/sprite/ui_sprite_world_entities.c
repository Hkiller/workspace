#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_sprite_world_i.h"
#include "ui_sprite_entity_it_i.h"

struct ui_sprite_world_entity_it_data {
    mem_allocrator_t m_alloc;
    struct cpe_hash_it m_entity_it;
};

static ui_sprite_entity_t ui_sprite_world_entity_it_next(ui_sprite_entity_it_t it) {
    struct ui_sprite_world_entity_it_data * data = (void*)(it + 1);
    ui_sprite_entity_t entity;

    for(entity = cpe_hash_it_next(&data->m_entity_it);
        entity;
        entity = cpe_hash_it_next(&data->m_entity_it))
    {
        if (!entity->m_is_wait_destory) return entity;
    }

    return NULL;
}

static void ui_sprite_world_entity_it_free(ui_sprite_entity_it_t it) {
    struct ui_sprite_world_entity_it_data * data = (void*)(it + 1);
    mem_free(data->m_alloc, it);
}

ui_sprite_entity_it_t
ui_sprite_world_entities(mem_allocrator_t alloc, ui_sprite_world_t world) {
    ui_sprite_entity_it_t new_it;
    struct ui_sprite_world_entity_it_data * new_data;

    new_it = mem_alloc(
        alloc,
        sizeof(struct ui_sprite_entity_it)
        + sizeof(struct ui_sprite_world_entity_it_data));
    if (new_it == NULL) return NULL;

    new_it->m_next = ui_sprite_world_entity_it_next;
    new_it->m_free = ui_sprite_world_entity_it_free;

    new_data = (void*)(new_it + 1);
    new_data->m_alloc = alloc;
    cpe_hash_it_init(&new_data->m_entity_it, &world->m_entity_by_id);

    return new_it;
}

