#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "ui_sprite_world_i.h"
#include "ui_sprite_entity_i.h"
#include "ui_sprite_group_i.h"
#include "ui_sprite_world_res_i.h"
#include "ui_sprite_world_data_i.h"
#include "ui_sprite_event_i.h"
#include "ui_sprite_attr_monitor_i.h"
#include "ui_sprite_event_meta_i.h"
#include "ui_sprite_component_i.h"
#include "ui_sprite_component_meta_i.h"

ui_sprite_world_t ui_sprite_world_create(ui_sprite_repository_t repo, const char * name) {
    ui_sprite_world_t world;
    uint16_t i;

    world = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_world));
    if (world == NULL) {
        CPE_ERROR(repo->m_em, "create world: alloc fail!");
        return NULL;
    }

    world->m_repo = repo;
    cpe_str_dup(world->m_name, sizeof(world->m_name), name);
    world->m_max_entity_id = 0;
    world->m_in_tick = 0;
	world->m_tick_adj = 1.0f;
    TAILQ_INIT(&world->m_resources_by_order);
    TAILQ_INIT(&world->m_wait_destory_entities);
    TAILQ_INIT(&world->m_pending_monitors);
    TAILQ_INIT(&world->m_pending_events);
    TAILQ_INIT(&world->m_datas);
    world->m_updator_count = 0;

    TAILQ_INIT(&world->m_updating_components);
    for(i = 0; i < CPE_ARRAY_SIZE(world->m_updating_nodes); ++i) {
        TAILQ_INIT(&world->m_updating_nodes[i].m_components);
    }

    world->m_evt_processed_entities_buf = NULL;

    if (cpe_hash_table_init(
            &world->m_entity_by_id,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_entity_id_hash,
            (cpe_hash_eq_t) ui_sprite_entity_id_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_entity, m_hh_for_id),
            -1) != 0)
    {
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_entity_by_name,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_entity_name_hash,
            (cpe_hash_eq_t) ui_sprite_entity_name_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_entity, m_hh_for_name),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_entity_by_id);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_entity_protos,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_entity_name_hash,
            (cpe_hash_eq_t) ui_sprite_entity_name_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_entity, m_hh_for_name),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_resources,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_world_res_hash,
            (cpe_hash_eq_t) ui_sprite_world_res_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_world_res, m_hh_for_world),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_event_handlers,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_event_handler_hash,
            (cpe_hash_eq_t) ui_sprite_event_handler_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_event_handler, m_hh_for_world),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

	if (cpe_hash_table_init(
            &world->m_attr_monitor_bindings,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_attr_monitor_binding_hash,
            (cpe_hash_eq_t) ui_sprite_attr_monitor_binding_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_attr_monitor_binding, m_hh_for_world),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_event_handlers);
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_group_by_name,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_group_name_hash,
            (cpe_hash_eq_t) ui_sprite_group_name_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_group, m_hh_for_name),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_attr_monitor_bindings);
        cpe_hash_table_fini(&world->m_event_handlers);
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_group_by_id,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_group_id_hash,
            (cpe_hash_eq_t) ui_sprite_group_id_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_group, m_hh_for_id),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_group_by_name);
        cpe_hash_table_fini(&world->m_attr_monitor_bindings);
        cpe_hash_table_fini(&world->m_event_handlers);
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (ui_sprite_world_add_updator(world, ui_sprite_world_update_components, world) != 0) {
        cpe_hash_table_fini(&world->m_group_by_id);
        cpe_hash_table_fini(&world->m_group_by_name);
        cpe_hash_table_fini(&world->m_attr_monitor_bindings);
        cpe_hash_table_fini(&world->m_event_handlers);
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (ui_sprite_world_start_tick(world) != 0) {
        cpe_hash_table_fini(&world->m_group_by_id);
        cpe_hash_table_fini(&world->m_group_by_name);
        cpe_hash_table_fini(&world->m_attr_monitor_bindings);
        cpe_hash_table_fini(&world->m_event_handlers);
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&repo->m_worlds, world, m_next_for_repo);

    return world;
}

static void ui_sprite_world_clear_event_handler(ui_sprite_world_t world) {
    struct cpe_hash_it event_handler_it;
    ui_sprite_event_handler_t event_handler;

    cpe_hash_it_init(&event_handler_it, &world->m_event_handlers);

    event_handler = cpe_hash_it_next(&event_handler_it);
    while (event_handler) {
        ui_sprite_event_handler_t next = cpe_hash_it_next(&event_handler_it);

        ui_sprite_event_handler_free(world, event_handler);

        event_handler = next;
    }
}

static void ui_sprite_world_clear_attr_monitor(ui_sprite_world_t world) {
    while(cpe_hash_table_count(&world->m_attr_monitor_bindings) > 0) {
        struct cpe_hash_it attr_monitor_binding_it;
        ui_sprite_attr_monitor_binding_t attr_monitor_binding;

        cpe_hash_it_init(&attr_monitor_binding_it, &world->m_attr_monitor_bindings);

        attr_monitor_binding = cpe_hash_it_next(&attr_monitor_binding_it);
        assert(attr_monitor_binding);

        ui_sprite_attr_monitor_free(world, attr_monitor_binding->m_monitor);
    }
}

ui_sprite_world_t ui_sprite_world_find(ui_sprite_repository_t repo, const char * name) {
    ui_sprite_world_t world;

    TAILQ_FOREACH(world, &repo->m_worlds, m_next_for_repo) {
        if (strcmp(world->m_name, name) == 0) return world;
    }

    return NULL;
}

const char * ui_sprite_world_name(ui_sprite_world_t world) {
    return world->m_name;
}

gd_app_context_t ui_sprite_world_app(ui_sprite_world_t world) {
    return world->m_repo->m_app;
}

void ui_sprite_world_free(ui_sprite_world_t world) {
    ui_sprite_repository_t repo = world->m_repo;
    uint16_t i;

    ui_sprite_world_stop_tick(world);

    ui_sprite_entity_free_all(world);
    ui_sprite_entity_proto_free_all(world);
    ui_sprite_group_free_all(world);
    ui_sprite_world_res_free_all(world);
    ui_sprite_world_clear_event_handler(world);
    ui_sprite_world_clear_attr_monitor(world);
    ui_sprite_world_data_free_all(world);

    assert(TAILQ_EMPTY(&world->m_pending_monitors));

    while(!TAILQ_EMPTY(&world->m_pending_events)) {
        ui_sprite_pending_event_free(world, TAILQ_FIRST(&world->m_pending_events));
    }

    assert(TAILQ_EMPTY(&world->m_wait_destory_entities));

    assert(TAILQ_EMPTY(&world->m_pending_events));
    assert(TAILQ_EMPTY(&world->m_updating_components));
    for(i = 0; i < CPE_ARRAY_SIZE(world->m_updating_nodes); ++i) {
        assert(TAILQ_EMPTY(&world->m_updating_nodes[i].m_components));
    }

    cpe_hash_table_fini(&world->m_entity_by_id);
    cpe_hash_table_fini(&world->m_entity_by_name);
    cpe_hash_table_fini(&world->m_entity_protos);
    cpe_hash_table_fini(&world->m_resources);
    cpe_hash_table_fini(&world->m_event_handlers);
    cpe_hash_table_fini(&world->m_group_by_id);
    cpe_hash_table_fini(&world->m_group_by_name);
    cpe_hash_table_fini(&world->m_attr_monitor_bindings);

    if (world->m_evt_processed_entities_buf) {
        mem_free(repo->m_alloc, world->m_evt_processed_entities_buf);
    }

    TAILQ_REMOVE(&repo->m_worlds, world, m_next_for_repo);
    
    mem_free(repo->m_alloc, world);
}

ui_sprite_repository_t ui_sprite_world_repository(ui_sprite_world_t world) {
    return world->m_repo;
}

xcomputer_t ui_sprite_world_computer(ui_sprite_world_t world) {
    return world->m_repo->m_computer;
}

void ui_sprite_world_send_event(ui_sprite_world_t world, LPDRMETA meta, void const * data, size_t size) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_pending_event_t processing_evt =
        ui_sprite_event_enqueue(
            world, NULL, 
            ui_sprite_repository_event_debug_level(repo, meta), meta, data, size);
    if (processing_evt == NULL) {
        CPE_ERROR(repo->m_em, "world send event: event enqueue fail!");
    }

    processing_evt->m_target_count = 1;
    processing_evt->m_targets[0].m_type = ui_sprite_event_target_type_world;
}

void ui_sprite_world_send_event_to(ui_sprite_world_t world, const char * input_targets, LPDRMETA meta, void const * data, size_t size) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_pending_event_t processing_evt;
    char * targets;

    processing_evt = ui_sprite_event_enqueue(
        world, NULL,
        ui_sprite_repository_event_debug_level(repo, meta), meta, data, size);
    if (processing_evt == NULL) {
        CPE_ERROR(repo->m_em, "world send event: event enqueue fail!");
		return;
    }

    targets = cpe_str_mem_dup(repo->m_alloc, input_targets);

    if (ui_sprite_event_analize_targets(processing_evt, world, NULL,  targets, NULL) != 0) {
        ui_sprite_pending_event_free(world, processing_evt);
    }

    mem_free(repo->m_alloc, targets);
}

void ui_sprite_world_build_and_send_event(
    ui_sprite_world_t world, const char * event_def, dr_data_source_t data_source)
{
    ui_sprite_event_build_and_enqueue(world, NULL, event_def, data_source, 0);
}

ui_sprite_event_handler_t
ui_sprite_world_add_event_handler(
    ui_sprite_world_t world, const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx)
{
    ui_sprite_repository_t repo = world->m_repo;

    ui_sprite_event_handler_t handler =
        ui_sprite_event_handler_create(world, NULL, event_name, 0, fun, ctx);
    if (handler == NULL) {
        CPE_ERROR(repo->m_em, "world add handler of event %s fail!", event_name);
        return NULL;
    }

    return handler;
}

void ui_sprite_world_clear_event_handler_by_ctx(ui_sprite_world_t world, void * ctx) {
    struct cpe_hash_it event_handler_it;
    ui_sprite_event_handler_t event_handler;

    cpe_hash_it_init(&event_handler_it, &world->m_event_handlers);

    event_handler = cpe_hash_it_next(&event_handler_it);
    while (event_handler) {
        ui_sprite_event_handler_t next = cpe_hash_it_next(&event_handler_it);

        if (event_handler->m_ctx == ctx) {
            ui_sprite_event_handler_free(world, event_handler);
        }

        event_handler = next;
    }
}

static int ui_sprite_world_cmp_updator(const void * i_l, const void * i_r) {
    return (int)(((ui_sprite_world_updator_t)i_l)->m_update_priority)
        - (int)(((ui_sprite_world_updator_t)i_r)->m_update_priority);
}

int ui_sprite_world_add_updator(ui_sprite_world_t world, ui_sprite_world_update_fun_t fun, void * ctx) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_world_updator_t updator;

    if (world->m_updator_count + 1 > CPE_ARRAY_SIZE(world->m_updators)) {
        CPE_ERROR(repo->m_em, "ui_sprite_world_update_crate: alloc fail!");
        return -1;
    }

    updator = world->m_updators + world->m_updator_count;
    world->m_updator_count++;

    updator->m_update_priority = 0;
    updator->m_fun = fun;
    updator->m_ctx = ctx;

    qsort(world->m_updators, world->m_updator_count, sizeof(world->m_updators[0]), ui_sprite_world_cmp_updator);

    return 0;
}

void ui_sprite_world_remove_updator(ui_sprite_world_t world, void * ctx) {
    uint8_t i;

    for(i = 0; i < world->m_updator_count; ++i) {
        ui_sprite_world_updator_t updator = world->m_updators + i;

        if (updator->m_ctx == ctx) {
            memmove(updator, updator + 1, sizeof(*updator) * (world->m_updator_count - i - 1));
            --world->m_updator_count;
            return;
        }
    }
}

int ui_sprite_world_set_updator_priority(ui_sprite_world_t world, void * ctx, int8_t priority) {
    uint8_t i;

    for(i = 0; i < world->m_updator_count; ++i) {
        ui_sprite_world_updator_t updator = world->m_updators + i;
        if (updator->m_ctx == ctx) {
            updator->m_update_priority = priority;
            qsort(world->m_updators, world->m_updator_count, sizeof(world->m_updators[0]), ui_sprite_world_cmp_updator);
            return 0;
        }
    }

    return -1;
}

void ui_sprite_world_update_components(ui_sprite_world_t world, void * ctx, float delta_s) {
    ui_sprite_update_node_t update_node;

    /*清理等待删除的entity */
    ui_sprite_entity_clear_destoried(world);

    /*    首先标记所有需要处理的更新 */
    TAILQ_FOREACH(update_node, &world->m_updating_components, m_next) {
        ui_sprite_component_t component;
        TAILQ_FOREACH(component, &update_node->m_components, m_next_for_updating) {
            component->m_need_process = 1;
        }
    }

    /*    处理所有标记过的component */
    for(update_node = TAILQ_FIRST(&world->m_updating_components);
        update_node != TAILQ_END(&world->m_updating_components);
        )
    {
        ui_sprite_update_node_t next_node = TAILQ_NEXT(update_node, m_next);
        ui_sprite_component_t component;

        for(component = TAILQ_FIRST(&update_node->m_components);
            component != TAILQ_END(&update_node->m_components);
            )
        {
            ui_sprite_component_t next = TAILQ_NEXT(component, m_next_for_updating);

            if (component->m_need_process) {
                component->m_need_process = 0;

				assert(component->m_meta->m_update_fun);
				component->m_meta->m_update_fun(component, component->m_meta->m_update_fun_ctx, delta_s);
				ui_sprite_event_dispatch(world);
            }
        
            component = next;
        }

        update_node = next_node;
    }

    /*清理等待删除的entity */
    ui_sprite_entity_clear_destoried(world);
}

void ui_sprite_world_tick(ui_sprite_world_t world, float delta_s) {
    float delta_s_adj;
    uint8_t i;

	delta_s_adj = delta_s * world->m_tick_adj;

    for(i = 0; i < world->m_updator_count; ++i) {
        ui_sprite_world_updator_t updator = world->m_updators + i;
        updator->m_fun(world, updator->m_ctx, delta_s_adj);
    }
}

static ptr_int_t ui_sprite_world_app_tick(void * ctx, ptr_int_t arg, float delta_s) {
    ui_sprite_world_t world = ctx;
	float delta_s_adj;
    ptr_int_t processed_count = 0;
    uint8_t i;

	delta_s_adj = delta_s * world->m_tick_adj;

    for(i = 0; i < world->m_updator_count; ++i) {
        ui_sprite_world_updator_t updator = world->m_updators + i;
        
        updator->m_fun(world, updator->m_ctx, delta_s_adj);
        ++processed_count;
    }

    return processed_count;
}

uint8_t ui_sprite_world_is_tick_start(ui_sprite_world_t world) {
    return world->m_in_tick;
}

int ui_sprite_world_start_tick(ui_sprite_world_t world) {
    int r;

    if (world->m_in_tick) return 0;

    r = gd_app_tick_add(world->m_repo->m_app, ui_sprite_world_app_tick, world, 0);

    if (r == 0) world->m_in_tick = 1;

    return r;
}

void ui_sprite_world_stop_tick(ui_sprite_world_t world) {
    if (!world->m_in_tick) return;

    if (gd_app_tick_remove(world->m_repo->m_app, ui_sprite_world_app_tick, world) == 0) {
        world->m_in_tick = 0;
    }
}

float ui_sprite_world_tick_adj(ui_sprite_world_t world) {
	return world->m_tick_adj;
}

void ui_sprite_world_set_tick_adj(ui_sprite_world_t world, float tick_adj) {
	world->m_tick_adj = tick_adj;
}

void ui_sprite_component_enqueue(ui_sprite_world_t world, ui_sprite_component_t component, int8_t priority) {
    struct ui_sprite_update_node * node = world->m_updating_nodes + ((int)priority + 128);

    if (TAILQ_EMPTY(&node->m_components)) {
        TAILQ_INSERT_TAIL(&world->m_updating_components, node, m_next);
    }

    TAILQ_INSERT_TAIL(&node->m_components, component, m_next_for_updating);
}

void ui_sprite_component_dequeue(ui_sprite_world_t world, ui_sprite_component_t component, int8_t priority) {
    struct ui_sprite_update_node * node = world->m_updating_nodes + ((int)priority + 128);

    TAILQ_REMOVE(&node->m_components, component, m_next_for_updating);

    if (TAILQ_EMPTY(&node->m_components)) {
        TAILQ_REMOVE(&world->m_updating_components, node, m_next);
    }
}
