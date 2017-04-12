#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/pal/pal_strings.h"
#include "ui_sprite_component_i.h"
#include "ui_sprite_component_meta_i.h"
#include "ui_sprite_entity_i.h"
#include "ui_sprite_event_meta_i.h"
#include "ui_sprite_attr_monitor_i.h"

static ui_sprite_component_t ui_sprite_component_create_i(ui_sprite_entity_t entity, ui_sprite_component_meta_t meta) {
    ui_sprite_repository_t repo = entity->m_world->m_repo;
    ui_sprite_component_t component;

    if (ui_sprite_component_find(entity, meta->m_name) != NULL) {
        CPE_ERROR(repo->m_em, "create component %s fail, duplicate!", meta->m_name);
        return NULL;
    }

    component = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_component) + meta->m_size);
    if (component == NULL) {
        CPE_ERROR(repo->m_em, "create component %s fail, alloc error!", meta->m_name);
        return NULL;
    }

    component->m_entity = entity;
    component->m_meta = meta;
    component->m_is_active = 0;
    component->m_is_update = 0;
    component->m_need_process = 0;
    TAILQ_INIT(&component->m_event_handlers);
    TAILQ_INIT(&component->m_attr_monitors);

    TAILQ_INSERT_TAIL(&entity->m_components, component, m_next_for_entity);
    TAILQ_INSERT_TAIL(&meta->m_components, component, m_next_for_meta);

    if (repo->m_debug || entity->m_debug) {
        CPE_INFO(
            repo->m_em, "entity %d(%s) add component %s",
            entity->m_id, entity->m_name, meta->m_name);
    }

    return component;
}

static void ui_sprite_component_free_i(ui_sprite_component_t component) {
    ui_sprite_component_meta_t meta = component->m_meta;
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_repository_t repo = entity->m_world->m_repo;

    if (component->m_is_active) {
        ui_sprite_component_exit(component);
    }
    assert(!component->m_is_update);

    ui_sprite_event_handler_clear_all(entity->m_world, &component->m_event_handlers);
    assert(TAILQ_EMPTY(&component->m_event_handlers));

    ui_sprite_attr_monitor_clear_all(entity->m_world, &component->m_attr_monitors);
    assert(TAILQ_EMPTY(&component->m_attr_monitors));

    TAILQ_REMOVE(&entity->m_components, component, m_next_for_entity);
    TAILQ_REMOVE(&meta->m_components, component, m_next_for_meta);

    if (repo->m_debug || entity->m_debug) {
        CPE_INFO(
            repo->m_em, "entity %d(%s) remove component %s",
            entity->m_id, entity->m_name, meta->m_name);
    }

    mem_free(repo->m_alloc, component);
}

ui_sprite_component_t ui_sprite_component_create(ui_sprite_entity_t entity, const char * type) {
    ui_sprite_repository_t repo = entity->m_world->m_repo;
    ui_sprite_component_meta_t meta;
    ui_sprite_component_t component;

    meta = ui_sprite_component_meta_find(repo, type);
    if (meta == NULL) {
        CPE_ERROR(repo->m_em, "create component %s fail, type is unknown!", type);
        return NULL;
    }

    component = ui_sprite_component_create_i(entity, meta);

    if (component) {
        if (meta->m_init_fun) {
            if (meta->m_init_fun(component, meta->m_init_fun_ctx) != 0) {
                CPE_ERROR(repo->m_em, "create component %s fail, init fail!", type);
                ui_sprite_component_free_i(component);
                return NULL;
            }
        }
        else {
            bzero(ui_sprite_component_data(component), component->m_meta->m_size);
        }
    }

    return component;
}

void ui_sprite_component_free(ui_sprite_component_t component) {
    ui_sprite_component_meta_t meta = component->m_meta;

    if (meta->m_free_fun) meta->m_free_fun(component, meta->m_free_fun_ctx);

    ui_sprite_component_free_i(component);
}

ui_sprite_component_t ui_sprite_component_clone(ui_sprite_entity_t entity, ui_sprite_component_t from) {
    ui_sprite_repository_t repo = entity->m_world->m_repo;
    ui_sprite_component_t to;

    to = ui_sprite_component_create_i(entity, from->m_meta);
    if (to == NULL) {
        CPE_ERROR(
            repo->m_em, "clone component %s: alloc fail!", from->m_meta->m_name);
        return NULL;
    }

    if (from->m_meta->m_copy_fun) {
        if (from->m_meta->m_copy_fun(to, from, from->m_meta->m_copy_fun_ctx) != 0) {
            CPE_ERROR(
                repo->m_em, "clone component %s: copy fail!", from->m_meta->m_name);
            ui_sprite_component_free_i(to);
            return NULL;
        }
    }
    else {
        memcpy(ui_sprite_component_data(to), ui_sprite_component_data(from), from->m_meta->m_size);
    }

    return to;
}

const char * ui_sprite_component_name(ui_sprite_component_t component) {
    return component->m_meta->m_name;
}

ui_sprite_component_meta_t ui_sprite_component_meta(ui_sprite_component_t component) {
    return component->m_meta;
}

void * ui_sprite_component_data(ui_sprite_component_t component) {
    return component + 1;
}

size_t ui_sprite_component_data_size(ui_sprite_component_t component) {
    return component->m_meta->m_size;
}

ui_sprite_component_t ui_sprite_component_from_data(void * data) {
    return ((ui_sprite_component_t)data) - 1;
}

ui_sprite_entity_t ui_sprite_component_entity(ui_sprite_component_t component) {
    return component->m_entity;
}

ui_sprite_component_t ui_sprite_component_find(ui_sprite_entity_t entity, const char * type) {
    ui_sprite_component_t component;

    TAILQ_FOREACH(component, &entity->m_components, m_next_for_entity) {
        if (strcmp(component->m_meta->m_name, type) == 0) return component;
    }

    return NULL;
}

int ui_sprite_component_enter(ui_sprite_component_t component) {
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_repository_t repo = entity->m_world->m_repo;

    if (component->m_is_active) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s) component %s already entered",
            entity->m_id, entity->m_name, component->m_meta->m_name);
        return -1;
    }

    if (repo->m_debug || entity->m_debug) {
        CPE_INFO(
            repo->m_em, "entity %d(%s) component %s enter", entity->m_id, entity->m_name, component->m_meta->m_name);
    }

    component->m_is_active = 1;

    if (component->m_meta->m_enter_fun) {
        if (component->m_meta->m_enter_fun(component, component->m_meta->m_enter_fun_ctx) != 0) {
            CPE_ERROR(
                repo->m_em, "entity %d(%s) component %s enter fail",
                entity->m_id, entity->m_name, component->m_meta->m_name);

            ui_sprite_attr_monitor_clear_all(entity->m_world, &component->m_attr_monitors);
            ui_sprite_event_handler_clear_all(entity->m_world, &component->m_event_handlers);
            assert(TAILQ_EMPTY(&component->m_event_handlers));

            component->m_is_active = 0;
            return -1;
        }
    }

    return 0;
}

uint8_t ui_sprite_component_is_active(ui_sprite_component_t component) {
    return component->m_is_active;
}

uint8_t ui_sprite_component_is_update(ui_sprite_component_t component) {
    return component->m_is_update;
}

void ui_sprite_component_send_event(
    ui_sprite_component_t component, LPDRMETA meta, void const * data, size_t size)
{
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_repository_t repo = entity->m_world->m_repo;
    ui_sprite_pending_event_t processing_evt =
        ui_sprite_event_enqueue(
            entity->m_world, entity, 
            ui_sprite_repository_event_debug_level(repo, meta),
            meta, data, size);

    if (processing_evt == NULL) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s) component %s: send event: event enqueue fail!",
            entity->m_id, entity->m_name, component->m_meta->m_name);
        return;
    }

    processing_evt->m_target_count = 1;
    processing_evt->m_targets[0].m_type = ui_sprite_event_target_type_entity;
    processing_evt->m_targets[0].m_data.to_entity_id = entity->m_id;
}

void ui_sprite_component_send_event_to(
    ui_sprite_component_t component, const char * input_targets, LPDRMETA meta, void const * data, size_t size)
{
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_repository_t repo = entity->m_world->m_repo;
    ui_sprite_pending_event_t processing_evt;
    char * targets;

    processing_evt =
        ui_sprite_event_enqueue(
            entity->m_world, entity, 
            ui_sprite_repository_event_debug_level(repo, meta), meta, data, size);

    if (processing_evt == NULL) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s) component %s: send event: event enqueue fail!",
            entity->m_id, entity->m_name, component->m_meta->m_name);
        return;
    }


    targets = cpe_str_mem_dup(repo->m_alloc, input_targets);

    if (ui_sprite_event_analize_targets(processing_evt, entity->m_world, entity,  targets, NULL) != 0) {
        ui_sprite_pending_event_free(entity->m_world, processing_evt);
    }

    mem_free(repo->m_alloc, targets);
}

void ui_sprite_component_build_and_send_event(
    ui_sprite_component_t component, const char * event_def, dr_data_source_t data_source)
{
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_event_build_and_enqueue(entity->m_world, entity, event_def, data_source, 0);
}

int ui_sprite_component_start_update(ui_sprite_component_t component) {
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_world_t world = entity->m_world;
    ui_sprite_repository_t repo = world->m_repo;

    if (component->m_is_update) return 0;

    if (component->m_meta->m_update_fun == NULL) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s) component %s not support update!",
            entity->m_id, entity->m_name, component->m_meta->m_name);
        return -1;
    }

    ui_sprite_component_enqueue(world, component, entity->m_update_priority);
    component->m_is_update = 1;
    component->m_need_process = 0;

    return 0;
}

void ui_sprite_component_stop_update(ui_sprite_component_t component) {
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_world_t world = entity->m_world;

    if (!component->m_is_update) return;

    ui_sprite_component_dequeue(world, component, entity->m_update_priority);
    component->m_is_update = 0;
    component->m_need_process = 0;
}

void ui_sprite_component_sync_update(ui_sprite_component_t component, uint8_t is_start) {
    if (is_start) {
        if (!component->m_is_update) {
            ui_sprite_component_start_update(component);
        }
    }
    else {
        if (component->m_is_update) {
            ui_sprite_component_stop_update(component);
        }
    }
}

void ui_sprite_component_exit(ui_sprite_component_t component) {
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_world_t world = entity->m_world;
    ui_sprite_repository_t repo = world->m_repo;

    if (!component->m_is_active) return;

    if (component->m_is_update) {
        ui_sprite_component_stop_update(component);
        assert(component->m_is_update == 0);
    }

    if (component->m_meta->m_exit_fun) {
        component->m_meta->m_exit_fun(component, component->m_meta->m_exit_fun_ctx);
    }

    ui_sprite_attr_monitor_clear_all(component->m_entity->m_world, &component->m_attr_monitors);
    ui_sprite_event_handler_clear_all(component->m_entity->m_world, &component->m_event_handlers);
    assert(TAILQ_EMPTY(&component->m_event_handlers));

    component->m_is_active = 0;

    if (repo->m_debug || entity->m_debug) {
        CPE_INFO(
            repo->m_em, "entity %d(%s) component %s exit", entity->m_id, entity->m_name, component->m_meta->m_name);
    }
}

ui_sprite_attr_monitor_t
ui_sprite_component_add_attr_monitor(ui_sprite_component_t component, const char * attrs, ui_sprite_attr_monitor_fun_t fun, void * ctx) {
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_world_t world = entity->m_world;
    ui_sprite_repository_t repo = world->m_repo;
	ui_sprite_attr_monitor_t monitor;

    if (!component->m_is_active) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s) component %s add attr monitor: component not active!",
            entity->m_id, entity->m_name, component->m_meta->m_name);
        return NULL;
    }

    monitor = ui_sprite_attr_monitor_create(world, &component->m_attr_monitors, entity, fun, ctx);
    if (monitor == NULL) {
        CPE_ERROR(
            repo->m_em, "component %s.%s: add attr monitor fail!",
            component->m_entity->m_name, component->m_meta->m_name);
        return NULL;
    }

    if (ui_sprite_attr_monitor_bind_attrs(world, entity, monitor, attrs) != 0) {
        CPE_ERROR(
            repo->m_em, "component %s.%s: bind attr monitor to %s fail!",
            component->m_entity->m_name, component->m_meta->m_name, attrs);
        ui_sprite_attr_monitor_free(world, monitor);
        return NULL;
    }
    
    return monitor;
}

ui_sprite_attr_monitor_t
ui_sprite_component_add_attr_monitor_by_def(ui_sprite_component_t component, const char * def, ui_sprite_attr_monitor_fun_t fun, void * ctx) {
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_world_t world = entity->m_world;
    ui_sprite_repository_t repo = world->m_repo;
	ui_sprite_attr_monitor_t monitor;

    if (!component->m_is_active) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s) component %s add attr monitor: component not active!",
            entity->m_id, entity->m_name, component->m_meta->m_name);
        return NULL;
    }

    monitor = ui_sprite_attr_monitor_create(world, &component->m_attr_monitors, entity, fun, ctx);
    if (monitor == NULL) {
        CPE_ERROR(
            repo->m_em, "component %s.%s: add attr monitor fail!",
            component->m_entity->m_name, component->m_meta->m_name);
        return NULL;
    }

    if (ui_sprite_attr_monitor_bind_by_def(world, entity, monitor, def) != 0) {
        CPE_ERROR(
            repo->m_em, "component %s.%s: bind attr monitor by def %s fail!",
            component->m_entity->m_name, component->m_meta->m_name, def);
        ui_sprite_attr_monitor_free(world, monitor);
        return NULL;
    }
    
    return monitor;
}

ui_sprite_attr_monitor_t
ui_sprite_component_add_attr_monitor_by_defs(
    ui_sprite_component_t component, const char * * defs, uint16_t def_count, ui_sprite_attr_monitor_fun_t fun, void * ctx)
{
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_world_t world = entity->m_world;
    ui_sprite_repository_t repo = world->m_repo;
	ui_sprite_attr_monitor_t monitor;
    uint16_t i;
    
    if (!component->m_is_active) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s) component %s add attr monitor: component not active!",
            entity->m_id, entity->m_name, component->m_meta->m_name);
        return NULL;
    }

    monitor = ui_sprite_attr_monitor_create(world, &component->m_attr_monitors, entity, fun, ctx);
    if (monitor == NULL) {
        CPE_ERROR(
            repo->m_em, "component %s.%s: add attr monitor fail!",
            component->m_entity->m_name, component->m_meta->m_name);
        return NULL;
    }

    for(i = 0; i < def_count; ++i) {
        if (ui_sprite_attr_monitor_bind_by_def(world, entity, monitor, defs[i]) != 0) {
            CPE_ERROR(
                repo->m_em, "component %s.%s: bind attr monitor by def %s fail!",
                component->m_entity->m_name, component->m_meta->m_name, defs[i]);
            ui_sprite_attr_monitor_free(world, monitor);
            return NULL;
        }
    }

    return monitor;
}

void ui_sprite_component_clear_attr_monitors(ui_sprite_component_t component) {
    ui_sprite_attr_monitor_t monitor;
    ui_sprite_attr_monitor_t next;
    
    for(monitor = TAILQ_FIRST(&component->m_attr_monitors); monitor; monitor = next) {
        next = TAILQ_NEXT(monitor, m_next_for_manage);

        if (monitor->m_state == ui_sprite_attr_monitor_working) {
            monitor->m_state = ui_sprite_attr_monitor_deleting;
        }
        else if (monitor->m_state == ui_sprite_attr_monitor_deleting) {
        }
        else {
            ui_sprite_attr_monitor_free(component->m_entity->m_world, monitor);
        }
    }
}

ui_sprite_event_handler_t ui_sprite_component_add_event_handler(
    ui_sprite_component_t component, ui_sprite_event_scope_t scope,
    const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx)
{
    ui_sprite_entity_t entity = component->m_entity;
    ui_sprite_world_t world = entity->m_world;
    ui_sprite_repository_t repo = world->m_repo;
	ui_sprite_event_handler_t handler;

    if (!component->m_is_active) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s) component %s add event: component not active!",
            entity->m_id, entity->m_name, component->m_meta->m_name);
        return NULL;
    }

    handler =
        ui_sprite_event_handler_create(
            world, &component->m_event_handlers,
            event_name,
            scope == ui_sprite_event_scope_self ? component->m_entity->m_id : 0,
            fun, ctx);
    if (handler == NULL) {
        CPE_ERROR(
            repo->m_em, "component %s.%s: add handler of event %s fail!",
            component->m_entity->m_name, component->m_meta->m_name, event_name);
        return NULL;
    }

    return handler;
}

ui_sprite_event_handler_t
ui_sprite_component_find_event_handler(
    ui_sprite_component_t component, const char * event_name)
{
    ui_sprite_event_handler_t handler;

    TAILQ_FOREACH(handler, &component->m_event_handlers, m_next_for_manage) {
        if (strcmp(handler->m_event_name, event_name) == 0) return handler;
    }

    return NULL;
}

void ui_sprite_component_remove_event_handler(
    ui_sprite_component_t component, const char * event_name)
{
    ui_sprite_event_handler_t handler;

    for(handler = TAILQ_FIRST(&component->m_event_handlers);
        handler != TAILQ_END(&component->m_event_handlers);
        )
    {
        ui_sprite_event_handler_t next = TAILQ_NEXT(handler, m_next_for_manage);
        
        if (strcmp(handler->m_event_name, event_name) == 0) {
            ui_sprite_event_handler_free(component->m_entity->m_world, handler);
        }

        handler = next;
    }
}

void ui_sprite_component_clear_event_handlers(ui_sprite_component_t component) {
    ui_sprite_event_handler_clear_all(component->m_entity->m_world, &component->m_event_handlers);
    assert(TAILQ_EMPTY(&component->m_event_handlers));
}

dr_data_t ui_sprite_component_attr_data(dr_data_t buff, ui_sprite_component_t component) {
    if (component->m_meta->m_data_meta == NULL) return NULL;

    buff->m_meta = component->m_meta->m_data_meta;
    buff->m_data = ((char*)ui_sprite_component_data(component)) + component->m_meta->m_data_start;
    buff->m_size = component->m_meta->m_data_size;

    return buff;
}

int ui_sprite_component_set_attr_data(ui_sprite_component_t component, dr_data_t data) {
    ui_sprite_repository_t repo = component->m_entity->m_world->m_repo;

    if (component->m_meta->m_data_meta == NULL) {
        CPE_ERROR(
            repo->m_em, "entry %d(%s): component %s: set attr data from %s(size=%d): component no data meta",
            component->m_entity->m_id, component->m_entity->m_name, component->m_meta->m_name,
            dr_meta_name(data->m_meta), (int)data->m_size);
        return -1;
    }

    if (data->m_meta == component->m_meta->m_data_meta) {
        assert(data->m_size == component->m_meta->m_data_size);
        memcpy(
            ((char*)ui_sprite_component_data(component)) + component->m_meta->m_data_start,
            data->m_data,
            component->m_meta->m_data_size);
        return 0;
    }
    else {
        if (dr_meta_copy_same_entry(
                ((char*)ui_sprite_component_data(component)) + component->m_meta->m_data_start,
                component->m_meta->m_data_size,
                component->m_meta->m_data_meta,
                data->m_data, data->m_size, data->m_meta,
                0, repo->m_em)
            <= 0)
        {
            CPE_ERROR(
                repo->m_em, "entry %d(%s): component %s: set attr data from %s(size=%d): copy same entry error",
                component->m_entity->m_id, component->m_entity->m_name, component->m_meta->m_name,
                dr_meta_name(data->m_meta), (int)data->m_size);
            return -1;
        }

        return 0;
    }
}
