#include <assert.h>
#include "ui_sprite_group_i.h"
#include "ui_sprite_group_binding_i.h"
#include "ui_sprite_event_meta_i.h"

ui_sprite_group_t ui_sprite_group_create(ui_sprite_world_t world, const char * name) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_group_t group;
    size_t name_len = (name ? strlen(name) : 0) + 1;

    group = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_group) + name_len);
    if (group == NULL) {
        CPE_ERROR(repo->m_em, "create group %s: alloc fail!", name);
        return NULL;
    }

    group->m_id = 0;
    group->m_world = world;
    group->m_name = (const char *)(group + 1);
    TAILQ_INIT(&group->m_join_groups);
    TAILQ_INIT(&group->m_elements);

    group->m_id = ++world->m_max_entity_id;

    cpe_hash_entry_init(&group->m_hh_for_id);
    if (cpe_hash_table_insert_unique(&world->m_group_by_id, group) != 0) {
        CPE_ERROR(repo->m_em, "create group %s: id %d duplicate!", name ? name : "?", group->m_id);
        mem_free(repo->m_alloc, group);
        return NULL;
    }

    if (name_len > 1) {
        assert(name);
        memcpy(group + 1, name, name_len);
        cpe_hash_entry_init(&group->m_hh_for_name);
        if (cpe_hash_table_insert_unique(&world->m_group_by_name, group) != 0) {
            CPE_ERROR(repo->m_em, "create group %s: name duplicate!", name);
            cpe_hash_table_remove_by_ins(&world->m_group_by_id, group);
            mem_free(repo->m_alloc, group);
            return NULL;
        }
    }
    else {
        * (char *)(group + 1) = 0;
    }
    
    return group;
}

void ui_sprite_group_free(ui_sprite_group_t group) {
    ui_sprite_world_t world = group->m_world;
    ui_sprite_repository_t repo = world->m_repo;

    while(!TAILQ_EMPTY(&group->m_join_groups)) {
        ui_sprite_group_binding_free(repo->m_alloc, TAILQ_FIRST(&group->m_join_groups));
    }

    while(!TAILQ_EMPTY(&group->m_elements)) {
        ui_sprite_group_binding_free(repo->m_alloc, TAILQ_FIRST(&group->m_elements));
    }

    if (group->m_name[0]) {
        cpe_hash_table_remove_by_ins(&world->m_group_by_name, group);
    }
    
    cpe_hash_table_remove_by_ins(&world->m_group_by_id, group);

    if (repo->m_debug) {
        CPE_INFO(repo->m_em, "group %d(%s) free", group->m_id, group->m_name);
    }

    mem_free(repo->m_alloc, group);
}

uint32_t ui_sprite_group_id(ui_sprite_group_t group) {
    return group->m_id;
}

const char * ui_sprite_group_name(ui_sprite_group_t group) {
    return group->m_name;
}

ui_sprite_world_t ui_sprite_group_world(ui_sprite_group_t gruop) {
    return gruop->m_world;
}

ui_sprite_group_t ui_sprite_group_find_by_id(ui_sprite_world_t world, uint32_t id) {
    struct ui_sprite_group key;
    key.m_id = id;
    return cpe_hash_table_find(&world->m_group_by_id, &key);
}

ui_sprite_group_t ui_sprite_group_find_by_name(ui_sprite_world_t world, const char * name) {
    struct ui_sprite_group key;
    key.m_name = name;
    return cpe_hash_table_find(&world->m_group_by_name, &key);
}

int ui_sprite_group_has_group(ui_sprite_group_t group, ui_sprite_group_t element) {
    ui_sprite_group_binding_t binding;

    TAILQ_FOREACH(binding, &element->m_join_groups, m_next_for_element) {
        if (binding->m_group == group) return 1;
    }

    return 0;
}

int ui_sprite_group_has_group_r(ui_sprite_group_t group, ui_sprite_group_t element) {
    ui_sprite_group_binding_t binding;

    TAILQ_FOREACH(binding, &group->m_elements, m_next_for_group) {
        if (binding->m_type == ui_sprite_group_binding_type_group) {
            if (binding->m_element.m_group == element) return 1;
            if (ui_sprite_group_has_group_r(binding->m_element.m_group, element)) return 1;
        }
    }

    return 0;
}

int ui_sprite_group_add_group(ui_sprite_group_t group, ui_sprite_group_t element) {
    ui_sprite_repository_t repo = group->m_world->m_repo;
    ui_sprite_group_binding_t binding;

    binding = ui_sprite_group_binding_create_group_group(repo->m_alloc, group, element);
    if (binding == NULL) return -1;

    return 0;
}

void ui_sprite_group_remove_group(ui_sprite_group_t group, ui_sprite_group_t element) {
    ui_sprite_repository_t repo = group->m_world->m_repo;
    ui_sprite_group_binding_t binding;

    TAILQ_FOREACH(binding, &element->m_join_groups, m_next_for_element) {
        if (binding->m_group == group) {
            ui_sprite_group_binding_free(repo->m_alloc, binding);
            return;
        }
    }
}

int ui_sprite_group_has_entity(ui_sprite_group_t group, ui_sprite_entity_t element) {
    ui_sprite_group_binding_t binding;

    TAILQ_FOREACH(binding, &element->m_join_groups, m_next_for_element) {
        if (binding->m_group == group) return 1;
    }

    return 0;
}

int ui_sprite_group_has_entity_r(ui_sprite_group_t group, ui_sprite_entity_t element) {
    ui_sprite_group_binding_t binding;

    TAILQ_FOREACH(binding, &group->m_elements, m_next_for_group) {
        if (binding->m_type == ui_sprite_group_binding_type_entity) {
            if (binding->m_element.m_entity == element) return 1;
        }
        else if (binding->m_type == ui_sprite_group_binding_type_group) {
            if (ui_sprite_group_has_entity_r(binding->m_element.m_group, element)) return 1;
        }
    }

    return 0;
}

int ui_sprite_group_add_entity(ui_sprite_group_t group, ui_sprite_entity_t element) {
    ui_sprite_repository_t repo = group->m_world->m_repo;
    ui_sprite_group_binding_t binding;

    binding = ui_sprite_group_binding_create_group_entity(repo->m_alloc, group, element);
    if (binding == NULL) return -1;

    return 0;
}

void ui_sprite_group_remove_entity(ui_sprite_group_t group, ui_sprite_entity_t element) {
    ui_sprite_repository_t repo = group->m_world->m_repo;
    ui_sprite_group_binding_t binding;

    TAILQ_FOREACH(binding, &element->m_join_groups, m_next_for_element) {
        if (binding->m_group == group) {
            ui_sprite_group_binding_free(repo->m_alloc, binding);
            return;
        }
    }
}

void ui_sprite_group_free_all(ui_sprite_world_t world) {
    struct cpe_hash_it group_it;
    ui_sprite_group_t group;

    cpe_hash_it_init(&group_it, &world->m_group_by_name);

    group = cpe_hash_it_next(&group_it);
    while (group) {
        ui_sprite_group_t next = cpe_hash_it_next(&group_it);
        ui_sprite_group_free(group);
        group = next;
    }
}

void ui_sprite_group_send_event(
    ui_sprite_group_t group,
    LPDRMETA meta, void const * data, size_t size)
{
    ui_sprite_repository_t repo = group->m_world->m_repo;
    ui_sprite_pending_event_t processing_evt =
        ui_sprite_event_enqueue(
            group->m_world, NULL, 
            ui_sprite_repository_event_debug_level(repo, meta), meta, data, size);
    if (processing_evt == NULL) {
        CPE_ERROR(
            repo->m_em, "group %d(%s): send event: event enqueue fail!",
            group->m_id, group->m_name);
        return;
    }

    processing_evt->m_target_count = 1;
    processing_evt->m_targets[0].m_type = ui_sprite_event_target_type_group;
    processing_evt->m_targets[0].m_data.to_group_id = group->m_id;
}

uint32_t ui_sprite_group_id_hash(const ui_sprite_group_t group) {
    return group->m_id;
}

int ui_sprite_group_id_eq(const ui_sprite_group_t l, const ui_sprite_group_t r) {
    return l->m_id == r->m_id;
}

uint32_t ui_sprite_group_name_hash(const ui_sprite_group_t group) {
    return cpe_hash_str(group->m_name, strlen(group->m_name));
}

int ui_sprite_group_name_eq(const ui_sprite_group_t l, const ui_sprite_group_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
