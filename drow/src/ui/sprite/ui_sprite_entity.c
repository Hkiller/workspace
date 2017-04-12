#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui_sprite_entity_i.h"
#include "ui_sprite_component_i.h"
#include "ui_sprite_component_meta_i.h"
#include "ui_sprite_group_i.h"
#include "ui_sprite_group_binding_i.h"
#include "ui_sprite_event_meta_i.h"
#include "ui_sprite_attr_monitor_i.h"

static ui_sprite_entity_t ui_sprite_entity_create_i(ui_sprite_world_t world, const char * name, uint8_t is_proto) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_entity_t entity;

    entity = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_entity));
    if (entity == NULL) {
        CPE_ERROR(repo->m_em, "create entity %s%s: alloc fail!", is_proto ? "proto " : "", name);
        return NULL;
    }

    entity->m_id = 0;
    entity->m_world = world;
    entity->m_is_proto = is_proto;
    entity->m_is_active = 0;
    entity->m_is_wait_destory = 0;
    entity->m_debug = 0;
    entity->m_update_priority = 0;
    entity->m_name = entity->m_name_buf;
    
    TAILQ_INIT(&entity->m_components);
    TAILQ_INIT(&entity->m_attr_monitors);
    TAILQ_INIT(&entity->m_event_handlers);
    TAILQ_INIT(&entity->m_join_groups);

    cpe_str_dup(entity->m_name_buf, sizeof(entity->m_name_buf), name);

    if (is_proto) {
        cpe_hash_entry_init(&entity->m_hh_for_name);
        if (cpe_hash_table_insert_unique(&world->m_entity_protos, entity) != 0) {
            CPE_ERROR(repo->m_em, "create entity proto %s: name duplicate!", name);
            mem_free(repo->m_alloc, entity);
            return NULL;
        }
    }
    else {
        entity->m_id = ++world->m_max_entity_id;
        cpe_hash_entry_init(&entity->m_hh_for_id);
        if (cpe_hash_table_insert_unique(&world->m_entity_by_id, entity) != 0) {
            CPE_ERROR(repo->m_em, "create entity %s: id %d duplicate!", name, entity->m_id);
            mem_free(repo->m_alloc, entity);
            return NULL;
        }

        if (name[0]) {
            cpe_hash_entry_init(&entity->m_hh_for_name);
            if (cpe_hash_table_insert_unique(&world->m_entity_by_name, entity) != 0) {
                CPE_ERROR(repo->m_em, "create entity %s: name duplicate!", name);
                cpe_hash_table_remove_by_ins(&world->m_entity_by_id, entity);
                mem_free(repo->m_alloc, entity);
                return NULL;
            }
        }
    }

    return entity;
}

ui_sprite_entity_t ui_sprite_entity_create(ui_sprite_world_t world, const char * name, const char * proto_type) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_entity_t proto = NULL;
    ui_sprite_entity_t entity;
    ui_sprite_component_t proto_component;

    if (proto_type) {
        proto = ui_sprite_entity_proto_find(world, proto_type);
        if (proto == NULL) {
            CPE_ERROR(repo->m_em, "create entity %s: proto %s not exist!", name, proto_type);
            return NULL;
        }
    }

    entity = ui_sprite_entity_create_i(world, name, 0);
    if (entity == NULL) return NULL;

    if (proto) {
        ui_sprite_group_binding_t old_group_binding;

        if (entity->m_name_buf[0] == 0) {
           snprintf(entity->m_name_buf, sizeof(entity->m_name_buf), "*%s", proto_type);
        }
        
        entity->m_debug = proto->m_debug;

        TAILQ_FOREACH(old_group_binding, &proto->m_join_groups, m_next_for_element) {
            ui_sprite_group_binding_t  binding = 
                ui_sprite_group_binding_create_group_entity(
                    repo->m_alloc, old_group_binding->m_group, entity);
            if (binding == NULL) {
                CPE_ERROR(
                    repo->m_em, "create entity %s: clone group binding %s fail!",
                    name, old_group_binding->m_group->m_name);
                ui_sprite_entity_free(entity);
                return NULL;
            }
        }

        TAILQ_FOREACH(proto_component, &proto->m_components, m_next_for_entity) {
            ui_sprite_component_t component = ui_sprite_component_clone(entity, proto_component);
            if (component == NULL) {
                CPE_ERROR(
                    repo->m_em, "create entity %s: clone component %s fail!",
                    name, proto_component->m_meta->m_name);
                ui_sprite_entity_free(entity);
                return NULL;
            }
        }
    }

    if (repo->m_debug || entity->m_debug) {
        if (proto_type) {
            CPE_INFO(repo->m_em, "entity %d(%s) create from proto %s", entity->m_id, entity->m_name, proto_type);
        }
        else {
            CPE_INFO(repo->m_em, "entity %d(%s) create", entity->m_id, entity->m_name);
        }
    }

    return entity;
}

void ui_sprite_entity_free(ui_sprite_entity_t entity) {
    ui_sprite_world_t world = entity->m_world;
    ui_sprite_repository_t repo = world->m_repo;

    if (entity->m_is_active) {
        ui_sprite_entity_exit(entity);
    }
    assert(!entity->m_is_active);

    if (entity->m_is_wait_destory) {
        TAILQ_REMOVE(&world->m_wait_destory_entities, entity, m_next_for_wait_destory);
        entity->m_is_wait_destory = 0;
    }

    while(!TAILQ_EMPTY(&entity->m_join_groups)) {
        ui_sprite_group_binding_free(repo->m_alloc, TAILQ_FIRST(&entity->m_join_groups));
    }

    while(!TAILQ_EMPTY(&entity->m_components)) {
        ui_sprite_component_free(TAILQ_LAST(&entity->m_components, ui_sprite_component_list));
    }

    ui_sprite_event_handler_clear_all(world, &entity->m_event_handlers);
    assert(TAILQ_EMPTY(&entity->m_event_handlers));

    ui_sprite_attr_monitor_clear_all(world, &entity->m_attr_monitors);
    assert(TAILQ_EMPTY(&entity->m_attr_monitors));

    if (entity->m_is_proto) {
        cpe_hash_table_remove_by_ins(&world->m_entity_protos, entity);
    }
    else {
        cpe_hash_table_remove_by_ins(&world->m_entity_by_id, entity);
        if (entity->m_name[0] && entity->m_name[0] != '*') {
            cpe_hash_table_remove_by_ins(&world->m_entity_by_name, entity);
        }
    }

    if (repo->m_debug) {
        CPE_INFO(repo->m_em, "entity %d(%s) free", entity->m_id, entity->m_name);
    }

    mem_free(repo->m_alloc, entity);
}

void ui_sprite_entity_free_all(ui_sprite_world_t world) {
    struct cpe_hash_it entity_it;
    ui_sprite_entity_t entity;

    cpe_hash_it_init(&entity_it, &world->m_entity_by_id);

    entity = cpe_hash_it_next(&entity_it);
    while (entity) {
        ui_sprite_entity_t next = cpe_hash_it_next(&entity_it);
        ui_sprite_entity_free(entity);
        entity = next;
    }
}

ui_sprite_entity_t ui_sprite_entity_find_by_id(ui_sprite_world_t world, uint32_t id) {
    struct ui_sprite_entity key;
    ui_sprite_entity_t r;
    key.m_id = id;
    r = cpe_hash_table_find(&world->m_entity_by_id, &key);

    return r
        ? (r->m_is_wait_destory ? NULL : r)
        : NULL;
}

ui_sprite_entity_t ui_sprite_entity_find_by_name(ui_sprite_world_t world, const char * name) {
    struct ui_sprite_entity key;
    ui_sprite_entity_t r;
    key.m_name = name;
    r = cpe_hash_table_find(&world->m_entity_by_name, &key);

    return r
        ? (r->m_is_wait_destory ? NULL : r)
        : NULL;
}

ui_sprite_entity_t ui_sprite_entity_find_auto_select(ui_sprite_world_t world, const char * def) {
    uint32_t entity_id;
    char * endp;
    entity_id = (uint32_t)strtol(def, &endp, 10);
    if (endp && *endp == 0) {
        return ui_sprite_entity_find_by_id(world, entity_id);
    }
    else {
        return ui_sprite_entity_find_by_name(world, def);
    }
}

uint32_t ui_sprite_entity_id(ui_sprite_entity_t entity) {
    return entity->m_id;
}

const char * ui_sprite_entity_name(ui_sprite_entity_t entity) {
    return entity->m_name;
}

void ui_sprite_entity_set_destory(ui_sprite_entity_t entity) {
    if (entity->m_is_wait_destory) return;

    TAILQ_INSERT_TAIL(&entity->m_world->m_wait_destory_entities, entity, m_next_for_wait_destory);
    entity->m_is_wait_destory = 1;
}

ui_sprite_world_t ui_sprite_entity_world(ui_sprite_entity_t entity) {
    return entity->m_world;
}

uint8_t ui_sprite_entity_debug(ui_sprite_entity_t entity) {
    return entity->m_debug;
}

void ui_sprite_entity_set_debug(ui_sprite_entity_t entity, uint8_t debug) {
    entity->m_debug = debug;
}

int8_t ui_sprite_entity_update_priority(ui_sprite_entity_t entity) {
    return entity->m_update_priority;
}

void ui_sprite_entity_set_update_priority(ui_sprite_entity_t entity, int8_t priority) {
    ui_sprite_component_t component;

    if (priority == entity->m_update_priority) return;

    if (entity->m_is_active) {
        ui_sprite_world_t world = entity->m_world;

        TAILQ_FOREACH(component, &entity->m_components, m_next_for_entity) {
            if (!component->m_is_update) continue;

            ui_sprite_component_dequeue(world, component, entity->m_update_priority);
            ui_sprite_component_enqueue(world, component, priority);
        }    
    }

    entity->m_update_priority = priority; 
}

ui_sprite_entity_t ui_sprite_entity_proto_create(ui_sprite_world_t world, const char * proto_name) {
    return ui_sprite_entity_create_i(world, proto_name, 1);
}

ui_sprite_entity_t ui_sprite_entity_proto_find(ui_sprite_world_t world, const char * proto_name) {
    struct ui_sprite_entity key;
    key.m_name = proto_name;
    return cpe_hash_table_find(&world->m_entity_protos, &key);
}

ui_sprite_event_handler_t ui_sprite_entity_add_event_handler(
    ui_sprite_entity_t entity, ui_sprite_event_scope_t scope,
    const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx)
{
    ui_sprite_world_t world = entity->m_world;
    ui_sprite_repository_t repo = world->m_repo;
	ui_sprite_event_handler_t handler;

    if (!entity->m_is_active) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s) add event: entity not active!",
            entity->m_id, entity->m_name);
        return NULL;
    }

    handler =
        ui_sprite_event_handler_create(
            world, &entity->m_event_handlers,
            event_name,
            scope == ui_sprite_event_scope_self ? entity->m_id : 0,
            fun, ctx);
    if (handler == NULL) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s): add handler of event %s fail!",
            entity->m_id, entity->m_name, event_name);
        return NULL;
    }

    return handler;
}

void ui_sprite_entity_send_event(
    ui_sprite_entity_t entity, LPDRMETA meta, void const * data, size_t size)
{
    ui_sprite_repository_t repo = entity->m_world->m_repo;
    ui_sprite_pending_event_t processing_evt =
        ui_sprite_event_enqueue(
            entity->m_world, NULL,
            ui_sprite_repository_event_debug_level(repo, meta), meta, data, size);
    if (processing_evt == NULL) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s): send event: event enqueue fail!",
            entity->m_id, entity->m_name);
        return;
    }

    processing_evt->m_target_count = 1;
    processing_evt->m_targets[0].m_type = ui_sprite_event_target_type_entity;
    processing_evt->m_targets[0].m_data.to_entity_id = entity->m_id;
}

void ui_sprite_entity_build_and_send_event(
    ui_sprite_entity_t entity, const char * event_def, dr_data_source_t data_source)
{
    ui_sprite_event_build_and_enqueue(entity->m_world, entity, event_def, data_source, 0);
}

void ui_sprite_entity_check_build_and_send_event(
    ui_sprite_entity_t entity, const char * event_def, dr_data_source_t data_source)
{
    ui_sprite_event_build_and_enqueue(entity->m_world, entity, event_def, data_source, 1);
}

ui_sprite_event_t
ui_sprite_entity_build_event(ui_sprite_entity_t entity, mem_allocrator_t alloc, const char * input_event_def, dr_data_source_t data_source) {
    ui_sprite_repository_t repo = entity->m_world->m_repo;
    char * def = cpe_str_mem_dup(repo->m_alloc, input_event_def);
    ui_sprite_event_t r;
    ui_sprite_event_meta_t event_meta;
    char * event_args;

    if (ui_sprite_event_analize_head(&event_meta, &event_args, repo, def, 0) != 0) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s): build and event: analize head fail: %s!",
            entity->m_id, entity->m_name, input_event_def);
        mem_free(repo->m_alloc, def);
        return NULL;
    }

    r = mem_alloc(alloc, sizeof(struct ui_sprite_event) + dr_meta_size(event_meta->m_meta));
    if (r == NULL) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s): build and event: alloc event fail: %s!",
            entity->m_id, entity->m_name, input_event_def);
        mem_free(repo->m_alloc, def);
        return NULL;
    }

    r->from_entity_id = entity->m_id;
    r->meta = event_meta->m_meta;
    r->size = dr_meta_size(event_meta->m_meta);
    r->data = r + 1;
    bzero(r + 1, r->size);

    if (ui_sprite_event_meta_build_event(r, repo, entity->m_world, entity, event_args, data_source) != 0) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s): build and event: build event fail: %s!",
            entity->m_id, entity->m_name, input_event_def);
        mem_free(repo->m_alloc, def);
        mem_free(alloc, r);
        return NULL;
    }
    
    mem_free(repo->m_alloc, def);

    return r;
}

void ui_sprite_entity_proto_free_all(ui_sprite_world_t world) {
    struct cpe_hash_it entity_it;
    ui_sprite_entity_t entity;

    cpe_hash_it_init(&entity_it, &world->m_entity_protos);

    entity = cpe_hash_it_next(&entity_it);
    while (entity) {
        ui_sprite_entity_t next = cpe_hash_it_next(&entity_it);
        ui_sprite_entity_free(entity);
        entity = next;
    }
}

uint8_t ui_sprite_entity_is_active(ui_sprite_entity_t entity) {
    return entity->m_is_active;
}

int ui_sprite_entity_enter(ui_sprite_entity_t entity) {
    ui_sprite_repository_t repo = entity->m_world->m_repo;
    ui_sprite_component_t component;

    if (entity->m_is_active) {
        CPE_ERROR(repo->m_em, "entity %d(%s) already enter!", entity->m_id, entity->m_name);
        return -1;
    }

    if (entity->m_is_proto) {
        CPE_ERROR(repo->m_em, "entity %d(%s) is proto, can`t enter!", entity->m_id, entity->m_name);
        return -1;
    }

    if (repo->m_debug || entity->m_debug) {
        CPE_INFO(repo->m_em, "entity %d(%s) enter", entity->m_id, entity->m_name);
    }

    entity->m_is_active = 1;

    TAILQ_FOREACH(component, &entity->m_components, m_next_for_entity) {
        if (ui_sprite_component_enter(component) != 0) {
            for(component = TAILQ_PREV(component, ui_sprite_component_list, m_next_for_entity);
                component != TAILQ_END(&entity->m_components);
                component = TAILQ_PREV(component, ui_sprite_component_list, m_next_for_entity))
            {
                ui_sprite_component_exit(component);
            }

            ui_sprite_event_handler_clear_all(entity->m_world, &entity->m_event_handlers);
            ui_sprite_attr_monitor_clear_all(entity->m_world, &entity->m_attr_monitors);
            entity->m_is_active = 0;
            return -1;
        }
    }


    return 0;
}

void ui_sprite_entity_exit(ui_sprite_entity_t entity) {
    ui_sprite_repository_t repo = entity->m_world->m_repo;
    ui_sprite_component_t component;

    if (!entity->m_is_active) return;

    TAILQ_FOREACH_REVERSE(component, &entity->m_components, ui_sprite_component_list, m_next_for_entity) {
        ui_sprite_component_exit(component);
    }

    ui_sprite_event_handler_clear_all(entity->m_world, &entity->m_event_handlers);

    entity->m_is_active = 0;

    if (repo->m_debug || entity->m_debug) {
        CPE_INFO(repo->m_em, "entity %d(%s) exit", entity->m_id, entity->m_name);
    }
}

void ui_sprite_entity_clear_destoried(ui_sprite_world_t world) {
    ui_sprite_repository_t repo = world->m_repo;

    while(!TAILQ_EMPTY(&world->m_wait_destory_entities)) {
        ui_sprite_entity_t destory_entity = TAILQ_FIRST(&world->m_wait_destory_entities);
        if (destory_entity->m_debug) {
            CPE_INFO(repo->m_em, "entity %d(%s): destory", destory_entity->m_id, destory_entity->m_name);
        }
        ui_sprite_entity_free(destory_entity);
    }
}

uint32_t ui_sprite_entity_id_hash(const ui_sprite_entity_t entity) {
    return entity->m_id;
}

int ui_sprite_entity_id_eq(const ui_sprite_entity_t l, const ui_sprite_entity_t r) {
    return l->m_id == r->m_id;
}

uint32_t ui_sprite_entity_name_hash(const ui_sprite_entity_t entity) {
    return cpe_hash_str(entity->m_name, strlen(entity->m_name));
}

int ui_sprite_entity_name_eq(const ui_sprite_entity_t l, const ui_sprite_entity_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
