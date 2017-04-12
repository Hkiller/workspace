#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/algorithm.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_data_entry.h"
#include "ui_sprite_event_i.h"
#include "ui_sprite_world_i.h"
#include "ui_sprite_entity_i.h"
#include "ui_sprite_group_i.h"
#include "ui_sprite_event_meta_i.h"
#include "ui_sprite_attr_monitor_i.h"

ui_sprite_event_handler_t
ui_sprite_event_handler_create(
    ui_sprite_world_t world, ui_sprite_event_handler_list_t * manage,
    const char * event_name, uint32_t event_entity,
    ui_sprite_event_process_fun_t fun, void * ctx)
{
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_event_handler_t handler = NULL;
    size_t name_len = strlen(event_name) + 1;

    handler = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_event_handler) + name_len);
    if (handler == NULL) {
        CPE_ERROR(repo->m_em, "crate event handler %s: alloc handler fail!", event_name);
        return NULL;
    }

    memcpy(handler + 1, event_name, name_len);

    handler->m_state = ui_sprite_event_handler_idle;
    handler->m_event_name = (const char *)(handler + 1);
    handler->m_event_entity = event_entity;
    handler->m_fun = fun;
    handler->m_ctx = ctx;
    handler->m_manage = manage;

    if (handler->m_manage) {
        TAILQ_INSERT_TAIL(handler->m_manage, handler, m_next_for_manage);
    }

    cpe_hash_entry_init(&handler->m_hh_for_world);
    if (cpe_hash_table_insert(&world->m_event_handlers, handler) != 0) {
        CPE_ERROR(repo->m_em, "crate event handler %s: insert to world fail!", event_name);
        
        if (handler->m_manage) {
            TAILQ_REMOVE(handler->m_manage, handler, m_next_for_manage);
        }

        mem_free(repo->m_alloc, handler);

        return NULL;
    }

    return handler;
}

void ui_sprite_event_handler_free(ui_sprite_world_t world, ui_sprite_event_handler_t handler) {
    ui_sprite_repository_t repo = world->m_repo;

    if (handler->m_state == ui_sprite_event_handler_working) {
        handler->m_state = ui_sprite_event_handler_deleting;
        return;
    }
        
    if (handler->m_manage) {
        TAILQ_REMOVE(handler->m_manage, handler, m_next_for_manage);
    }

    cpe_hash_table_remove_by_ins(&world->m_event_handlers, handler);

    mem_free(repo->m_alloc, handler);
}

const char * ui_sprite_event_handler_process_event(ui_sprite_event_handler_t handler) {
    return handler->m_event_name;
}

ui_sprite_event_process_fun_t ui_sprite_event_handler_process_fun(ui_sprite_event_handler_t handler) {
    return handler->m_fun;
}

void * ui_sprite_event_handler_process_ctx(ui_sprite_event_handler_t handler) {
    return handler->m_ctx;
}

int ui_sprite_event_add_target(
    ui_sprite_pending_event_t processing_evt,
    ui_sprite_entity_t from_entity,
    ui_sprite_world_t world, const char * str_target,
    dr_data_source_t data_source)
{
    ui_sprite_repository_t repo = world->m_repo;
    struct ui_sprite_event_target * target;

    if (processing_evt->m_target_count >= CPE_ARRAY_SIZE(processing_evt->m_targets)) {
        CPE_ERROR(repo->m_em, "add event target: target count overflow!");
        return -1;
    }

    target = &processing_evt->m_targets[processing_evt->m_target_count];

    if (str_target[0] == '@') {
        if (strcmp(str_target + 1, "self") == 0) {
            if (from_entity == NULL) {
                CPE_ERROR(repo->m_em, "send event to %s: event not send from entity", str_target);
                return -1;
            }
            else {
                processing_evt->m_target_count++;
                target->m_type = ui_sprite_event_target_type_entity;
                target->m_data.to_entity_id = ui_sprite_entity_id(from_entity);
                return 0;
            }
        }
        else if (strcmp(str_target, "world") == 0) {
            processing_evt->m_target_count++;
            target->m_type = ui_sprite_event_target_type_world;
            return 0;
        }
        else {
            char const * arg_name = str_target + 1;
            uint8_t is_group = 0;

            struct dr_data_entry from_attr_buf;
            dr_data_entry_t from_attr;
            uint32_t id;
            const char * name;
            ui_sprite_entity_t target_entity = NULL;
            ui_sprite_group_t target_group = NULL;

            if (*arg_name == '*') {
                is_group = 1;
                arg_name = arg_name + 1;
            }

            from_attr = dr_data_entry_search_in_source(&from_attr_buf, data_source, arg_name);
            if (from_attr == NULL && from_entity) {
                from_attr = ui_sprite_entity_find_attr(&from_attr_buf, from_entity, arg_name);
            }

            if (from_attr == NULL) {
                CPE_ERROR(repo->m_em, "send event to %s: attr %s not exist", str_target, arg_name);
                return -1;
            }

            if (dr_entry_try_read_uint32(&id, from_attr->m_data, from_attr->m_entry, repo->m_em) == 0) {
                if (is_group) {
                    target_group = ui_sprite_group_find_by_id(world, id);
                    if (target_group == NULL) {
                        CPE_ERROR(repo->m_em, "send event to %s: id from attr %s: group %d() not exist", str_target, arg_name, id);
                        return -1;
                    }
                }
                else {
                    target_entity = ui_sprite_entity_find_by_id(world, id);
                    if (target_entity == NULL) {
                        CPE_ERROR(repo->m_em, "send event to %s: id from attr %s: entity %d() not exist", str_target, arg_name, id);
                        return -1;
                    }
                }
            }
            else if ((name = dr_entry_read_string(from_attr->m_data, from_attr->m_entry))) {
                if (is_group) {
                    target_group = ui_sprite_group_find_by_name(world, name);
                    if (target_group == NULL) {
                        CPE_ERROR(repo->m_em, "send event to %s: id from attr %s: group (%s) not exist", str_target, arg_name, name);
                        return -1;
                    }
                }
                else {
                    target_entity = ui_sprite_entity_find_by_name(world, name);
                    if (target_entity == NULL) {
                        CPE_ERROR(repo->m_em, "send event to %s: id from attr %s: entity (%s) not exist", str_target, arg_name, name);
                        return -1;
                    }
                }
            }
            else {
                CPE_ERROR(repo->m_em, "send event to %s: attr %s convert to numeric or string fail", str_target, arg_name);
                return -1;
            }


            if (target_entity) {
                processing_evt->m_target_count++;
                target->m_type = ui_sprite_event_target_type_entity;
                target->m_data.to_entity_id = ui_sprite_entity_id(target_entity);
                return 0;
            }
            else {
                assert(target_group);
                processing_evt->m_target_count++;
                target->m_type = ui_sprite_event_target_type_group;
                target->m_data.to_group_id = ui_sprite_group_id(target_group);
                return 0;
            }
        }
    }
    else if (str_target[0] == '*') {
        ui_sprite_group_t group = ui_sprite_group_find_by_name(world, str_target + 1);
        if (group == NULL) {
            CPE_ERROR(repo->m_em, "send event to %s: unknown group", str_target + 1);
            return -1;
        }
        else {
            processing_evt->m_target_count++;
            target->m_type = ui_sprite_event_target_type_group;
            target->m_data.to_group_id = ui_sprite_group_id(group);
            return 0;
        }
    }
    else {
        ui_sprite_entity_t target_entity = ui_sprite_entity_find_by_name(world, str_target);
        if (target_entity) {
            processing_evt->m_target_count++;
            target->m_type = ui_sprite_event_target_type_entity;
            target->m_data.to_entity_id = ui_sprite_entity_id(target_entity);
            return 0;
        }
        else {
            CPE_ERROR(repo->m_em, "send event to %s: target not exist", str_target);
            return -1;
        }
    }

    assert(0);
    return -1;
}


int ui_sprite_event_analize_head(
    ui_sprite_event_meta_t * r_event_meta, char ** r_event_args,
    ui_sprite_repository_t repo, char * event_def, uint8_t ignore_not_exist)
{
    ui_sprite_event_meta_t event_meta;
    char * event_name;
    char * event_args;
    char * p;

    event_name = (char *)cpe_str_trim_head(event_def);
    if ((p = strchr(event_name, ':'))) {
        event_args = (char*)cpe_str_trim_head(p + 1);

        p = (char *)cpe_str_trim_tail(p, event_name);
        *p = 0;
    }
    else {
        event_args = cpe_str_trim_tail(event_name + strlen(event_name), event_name);
        event_args[0] = 0;
    }

    event_meta = ui_sprite_event_meta_find(repo, event_name);
    if (event_meta == NULL || event_meta->m_meta == NULL) {
        if (!ignore_not_exist) {
            CPE_ERROR(repo->m_em, "build event: event %s not exist!", event_name);
            return -1;
        }
        else {
            *r_event_meta = NULL;
            return 0;
        }
    }

    *r_event_meta = event_meta;
    *r_event_args = event_args;

    return 0;
}

int ui_sprite_event_analize_targets(
    ui_sprite_pending_event_t processing_evt,
    ui_sprite_world_t world, ui_sprite_entity_t from_entity, char * targets, dr_data_source_t data_source)
{
    char * sep;

    while((sep = strchr(targets, ','))) {
        char * target_name = targets;

        *cpe_str_trim_tail(sep, targets) = 0;
            
        targets = cpe_str_trim_head(sep + 1);

        if (target_name[0]) {
            if (ui_sprite_event_add_target(processing_evt, from_entity, world, target_name, data_source) != 0) return -1;
        }
    }

    if (targets[0]) {
        if (ui_sprite_event_add_target(processing_evt, from_entity, world, targets, data_source) != 0) return -1;
    }

    return 0;
}

static int ui_sprite_event_build_and_enqueue_i(
    ui_sprite_repository_t repo, ui_sprite_world_t world, ui_sprite_entity_t from_entity,
    char * event_def, dr_data_source_t data_source, uint8_t ignore_not_exist)
{
    ui_sprite_pending_event_t processing_evt;
    ui_sprite_event_meta_t event_meta;
    char * event_args;
    char * targets;
    char * event_data;

    targets = cpe_str_trim_head(event_def);
    if (*targets == '[') {
        char * end = strchr(targets, ']');
        if (end == NULL) {
            CPE_ERROR(repo->m_em, "send event: targets format error");
            return -1;
        }

        * cpe_str_trim_tail(end, targets) = 0;
        targets = cpe_str_trim_head(targets + 1);
        event_data = cpe_str_trim_head(end + 1);
    }
    else {
        event_data = targets;
        targets = NULL;
    }

    if (ui_sprite_event_analize_head(&event_meta, &event_args, repo, event_data, ignore_not_exist) != 0) {
        CPE_ERROR(repo->m_em, "send event: event format error");
        return -1;
    }

    if (event_meta == NULL) return 0;
    
    processing_evt = 
        ui_sprite_event_enqueue(
            world, from_entity,
            event_meta->m_debug_level, event_meta->m_meta, NULL, dr_meta_size(event_meta->m_meta));
    if (processing_evt == NULL) {
        CPE_ERROR(repo->m_em, "send event: enqueue event fail!");
        return -1;
    }
    processing_evt->m_debug_level = event_meta->m_debug_level;

    if (ui_sprite_event_meta_build_event(&processing_evt->m_data, repo, world, from_entity, event_args, data_source) != 0) {
        CPE_ERROR(repo->m_em, "send event: build event fail!");
        ui_sprite_pending_event_free(world, processing_evt);
        return -1;
    }

    if (targets == NULL) {
        if (from_entity == NULL) {
            CPE_ERROR(repo->m_em, "send event: can`t build event from world no targets!");
            ui_sprite_pending_event_free(world, processing_evt);
            return -1;
        }
        else {
            processing_evt->m_target_count = 1;
            processing_evt->m_targets[0].m_type = ui_sprite_event_target_type_entity;
            processing_evt->m_targets[0].m_data.to_entity_id = from_entity->m_id;
        }
    }
    else {
        if (ui_sprite_event_analize_targets(processing_evt, world, from_entity, targets, data_source) != 0) {
            ui_sprite_pending_event_free(world, processing_evt);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_event_build_and_enqueue(
    ui_sprite_world_t world, ui_sprite_entity_t from_entity,
    const char * input_def, dr_data_source_t data_source, uint8_t ignore_not_exist)
{
    ui_sprite_repository_t repo = world->m_repo;
    char * tmp_event = cpe_str_mem_dup(repo->m_alloc, input_def);
    char * event_def = tmp_event;
    char * p;

    event_def = cpe_str_trim_head(event_def);
    while((p = strchr(event_def, ';'))) {
        *p = 0;

        if (*event_def != 0) {
            if (ui_sprite_event_build_and_enqueue_i(repo, world, from_entity, event_def, data_source, ignore_not_exist) != 0) {
                CPE_ERROR(repo->m_em, "send event: %s fail!", input_def + (event_def - tmp_event));
            }
        }

        event_def = cpe_str_trim_head(p + 1);
    }

    if (*event_def != 0) {
        if (ui_sprite_event_build_and_enqueue_i(repo, world, from_entity, event_def, data_source, ignore_not_exist) != 0) {
            CPE_ERROR(repo->m_em, "send event: %s fail!", input_def + (event_def - tmp_event));
        }
    }
    
    mem_free(repo->m_alloc, tmp_event);
}

void ui_sprite_pending_event_free(ui_sprite_world_t world, ui_sprite_pending_event_t processing_evt) {
    TAILQ_REMOVE(&world->m_pending_events, processing_evt, m_next);
    mem_free(world->m_repo->m_alloc, processing_evt);
}

static void ui_sprite_event_do_send(
    ui_sprite_repository_t repo, ui_sprite_world_t world, uint32_t entity_id, ui_sprite_pending_event_t processing_evt)
{
    struct ui_sprite_event_handler key;
    ui_sprite_event_handler_t handler;

    key.m_event_name = dr_meta_name(processing_evt->m_data.meta);
    key.m_event_entity = entity_id;

    for(handler = cpe_hash_table_find(&world->m_event_handlers, &key);
        handler;
        )
    {
        ui_sprite_event_handler_t next;

        assert(handler->m_state == ui_sprite_event_handler_idle);

        
        handler->m_state = ui_sprite_event_handler_working;
        handler->m_fun(handler->m_ctx, &processing_evt->m_data);
        
        next = cpe_hash_table_find_next(&world->m_event_handlers, handler);
        if (handler->m_state == ui_sprite_event_handler_deleting) {
            ui_sprite_event_handler_free(world, handler);
        }
        else {
            assert(handler->m_state == ui_sprite_event_handler_working);
            handler->m_state = ui_sprite_event_handler_idle;
        }

        handler = next;
    }
}

static int ui_sprite_event_check_and_add_entities(ui_sprite_world_t world, uint32_t entity_id) {
    ui_sprite_repository_t repo = world->m_repo;
    cpe_sorted_vector_t v = &world->m_evt_processed_entities;

    if (world->m_evt_processed_entities_buf == NULL) {
        world->m_evt_processed_entities_buf = mem_alloc(repo->m_alloc, sizeof(entity_id) * 64);
        if (world->m_evt_processed_entities_buf == NULL) {
            CPE_ERROR(repo->m_em, "world: evt add processed evt: alloc fail!");
            return -1;
        }

        cpe_sorted_vector_init(v, world->m_evt_processed_entities_buf, 64, 0, sizeof(entity_id), cpe_comap_uint32);
    }
    else if (cpe_sorted_vector_is_full(v)) {
        size_t cur_size = cpe_sorted_vector_size(v);
        size_t new_capacity = cpe_sorted_vector_capacity(v) * 2;
        void * new_buff = mem_alloc(repo->m_alloc, sizeof(entity_id) * new_capacity);

        if (new_buff == NULL) {
            CPE_ERROR(repo->m_em, "world: evt add processed evt: alloc new fail, capacity=%d!", (int)new_capacity);
            return -1;
        }

        memcpy(new_buff, world->m_evt_processed_entities_buf, sizeof(entity_id) * cur_size);

        cpe_sorted_vector_init(v, new_buff, new_capacity, cur_size, sizeof(entity_id), cpe_comap_uint32);

        mem_free(repo->m_alloc, world->m_evt_processed_entities_buf);
        world->m_evt_processed_entities_buf = new_buff;
    }

    assert(!cpe_sorted_vector_is_full(v));

    return cpe_sorted_vector_insert_unique(v, &entity_id);
}

static void ui_sprite_event_dispatch_to_world(ui_sprite_world_t world, ui_sprite_pending_event_t processing_evt, const char * from_buf) {
    ui_sprite_repository_t repo = world->m_repo;
    struct cpe_hash_it entity_it;
    ui_sprite_entity_t entity;

    cpe_hash_it_init(&entity_it, &world->m_entity_by_id);

    entity = cpe_hash_it_next(&entity_it);
    while (entity) {
        ui_sprite_entity_t next = cpe_hash_it_next(&entity_it);

        if (entity->m_is_wait_destory) continue;

        if (ui_sprite_event_check_and_add_entities(world, ui_sprite_entity_id(entity)) != 0) continue;

        if (repo->m_debug >= processing_evt->m_debug_level
            || ui_sprite_entity_debug(entity) >= processing_evt->m_debug_level)
        {
            CPE_INFO(
                repo->m_em, "entity %d(%s): event %s: from world: %s%s",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                dr_meta_name(processing_evt->m_data.meta),
                from_buf,
                dr_json_dump_inline(
                    &repo->m_dump_buffer, 
                    processing_evt->m_data.data,
                    processing_evt->m_data.size,
                    processing_evt->m_data.meta));
        }
    
        ui_sprite_event_do_send(repo, world, entity->m_id, processing_evt);
        ui_sprite_event_do_send(repo, world, 0, processing_evt);

        entity = next;
    }

}

static void ui_sprite_event_dispatch_to_group(
    ui_sprite_world_t world, ui_sprite_pending_event_t processing_evt, uint32_t group_id, const char * from_buf)
{
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_group_t to_group = NULL;
    ui_sprite_entity_it_t entity_it;
    ui_sprite_entity_t to_entity = NULL;

    to_group = ui_sprite_group_find_by_id(world, group_id);
    if (to_group == NULL) {
        if (repo->m_debug >= processing_evt->m_debug_level) {
            CPE_INFO(
                repo->m_em, "gruop %d: ignore event (group not exist): %s: %s%s",
                group_id,
                dr_meta_name(processing_evt->m_data.meta),
                from_buf,
                dr_json_dump_inline(
                    &repo->m_dump_buffer, 
                    processing_evt->m_data.data,
                    processing_evt->m_data.size,
                    processing_evt->m_data.meta));
        }
        return;
    }

    entity_it = ui_sprite_group_entities(repo->m_alloc, to_group);
    if (entity_it == NULL) {
        CPE_ERROR(
            repo->m_em, "group %d(%s): ignore event (get eitities error): %s: %s%s",
            ui_sprite_group_id(to_group), ui_sprite_group_name(to_group),
            dr_meta_name(processing_evt->m_data.meta),
            from_buf,
            dr_json_dump_inline(
                &repo->m_dump_buffer, 
                processing_evt->m_data.data,
                processing_evt->m_data.size,
                processing_evt->m_data.meta));
        return;
    }

    while((to_entity = ui_sprite_entity_it_next(entity_it))) {
        if (to_entity->m_is_wait_destory) continue;
        if (ui_sprite_event_check_and_add_entities(world, ui_sprite_entity_id(to_entity))  != 0) continue;

        if (repo->m_debug  >= processing_evt->m_debug_level
            || ui_sprite_entity_debug(to_entity) >= processing_evt->m_debug_level)
        {
            CPE_INFO(
                repo->m_em, "entity %d(%s): event %s: from group %d(%s): %s%s",
                ui_sprite_entity_id(to_entity), ui_sprite_entity_name(to_entity),
                dr_meta_name(processing_evt->m_data.meta),
                ui_sprite_group_id(to_group), ui_sprite_group_name(to_group),
                from_buf,
                dr_json_dump_inline(
                    &repo->m_dump_buffer, 
                    processing_evt->m_data.data,
                    processing_evt->m_data.size,
                    processing_evt->m_data.meta));
        }

        ui_sprite_event_do_send(repo, world, to_entity->m_id, processing_evt);
        ui_sprite_event_do_send(repo, world, 0, processing_evt);
    }

    ui_sprite_entity_it_free(entity_it);
}

static void ui_sprite_event_dispatch_to_entity(
    ui_sprite_world_t world, ui_sprite_pending_event_t processing_evt, uint32_t entity_id, const char * from_buf)
{
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_entity_t to_entity = NULL;

    to_entity = ui_sprite_entity_find_by_id(world, entity_id);
    if (to_entity == NULL) {
        if (repo->m_debug >= processing_evt->m_debug_level) {
            CPE_INFO(
                repo->m_em, "entity %d: ignore event: %s: %s%s",
                entity_id,
                dr_meta_name(processing_evt->m_data.meta),
                from_buf,
                dr_json_dump_inline(
                    &repo->m_dump_buffer, 
                    processing_evt->m_data.data,
                    processing_evt->m_data.size,
                    processing_evt->m_data.meta));
        }
        return;
    }
    
    if (to_entity->m_is_wait_destory) return;

    if (ui_sprite_event_check_and_add_entities(world, ui_sprite_entity_id(to_entity)) != 0) return;

    if (repo->m_debug >= processing_evt->m_debug_level
        || ui_sprite_entity_debug(to_entity) >= processing_evt->m_debug_level)
    {
        CPE_INFO(
            repo->m_em, "entity %d(%s): event %s: %s%s",
            ui_sprite_entity_id(to_entity), ui_sprite_entity_name(to_entity),
            dr_meta_name(processing_evt->m_data.meta), from_buf,
            dr_json_dump_inline(
                &repo->m_dump_buffer, 
                processing_evt->m_data.data,
                processing_evt->m_data.size,
                processing_evt->m_data.meta));
    }

    ui_sprite_event_do_send(repo, world, to_entity->m_id, processing_evt);
    ui_sprite_event_do_send(repo, world, 0, processing_evt);
}

void ui_sprite_event_dispatch(ui_sprite_world_t world) {
    ui_sprite_repository_t repo = world->m_repo;

    /*处理属性变更 */
    ui_sprite_attr_monitor_process(world);


    while(!TAILQ_EMPTY(&world->m_pending_events)) {
        ui_sprite_pending_event_t processing_evt = TAILQ_FIRST(&world->m_pending_events);
        ui_sprite_entity_t from_entity = NULL;
        char from_buf[64];
        uint8_t i;

        TAILQ_REMOVE(&world->m_pending_events, processing_evt, m_next);

        /*清理已经处理过对象列表 */
        cpe_sorted_vector_set_size(&world->m_evt_processed_entities, 0);

        if (processing_evt->m_data.from_entity_id > 0) {
            from_entity = ui_sprite_entity_find_by_id(world, processing_evt->m_data.from_entity_id);
        }

        if (from_entity) {
            snprintf(
                from_buf, sizeof(from_buf), " from entity %d(%s):",
                ui_sprite_entity_id(from_entity), ui_sprite_entity_name(from_entity));
        }
        else {
            from_buf[0] = 0;
        }

        for(i = 0; i < processing_evt->m_target_count; ++i) {
            struct ui_sprite_event_target * target = &processing_evt->m_targets[i];

            if (target->m_type == ui_sprite_event_target_type_world) {
                ui_sprite_event_dispatch_to_world(world, processing_evt, from_buf);
            }
            else if (target->m_type == ui_sprite_event_target_type_group) {
                ui_sprite_event_dispatch_to_group(world, processing_evt, target->m_data.to_group_id, from_buf);
            }
            else if (target->m_type == ui_sprite_event_target_type_entity) {
                ui_sprite_event_dispatch_to_entity(world, processing_evt, target->m_data.to_entity_id, from_buf);
            }
            else {
                CPE_ERROR(
                    repo->m_em, "dispatch event %s: %sunknown target type %d",
                    dr_meta_name(processing_evt->m_data.meta), from_buf, target->m_type);
            }
        }

        mem_free(repo->m_alloc, processing_evt);

        /*处理属性变更 */
        ui_sprite_attr_monitor_process(world);
    }
}

ui_sprite_pending_event_t 
ui_sprite_event_enqueue(
    ui_sprite_world_t world, ui_sprite_entity_t from_entity,
    uint8_t debug_level, LPDRMETA meta, void const * data, size_t size)
{
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_pending_event_t delay_evt = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_pending_event) + size);
    if (delay_evt == NULL) {
        CPE_ERROR(repo->m_em, "send event %s: alloc fail, data size %d", dr_meta_name(meta), (int)size);
        return NULL;
    }

    delay_evt->m_debug_level = debug_level;
    delay_evt->m_target_count = 0;
    delay_evt->m_data.from_entity_id = from_entity ? from_entity->m_id : 0;
    delay_evt->m_data.meta = meta;
    delay_evt->m_data.data = delay_evt + 1;
    delay_evt->m_data.size = size;

    if (data) {
        memcpy(delay_evt + 1, data, size);
    }
    else {
        bzero(delay_evt + 1, size);
    }

    TAILQ_INSERT_TAIL(&world->m_pending_events, delay_evt, m_next);

    return delay_evt;
}

ui_sprite_event_t ui_sprite_event_copy(mem_allocrator_t alloc, ui_sprite_event_t evt) {
    ui_sprite_event_t result;

    result = mem_alloc(alloc, sizeof(struct ui_sprite_event) + evt->size);
    if (result == NULL) return NULL;

    result->from_entity_id = evt->from_entity_id;
    result->meta = evt->meta;
    result->data = result + 1;
    result->size = evt->size;

    memcpy(result + 1, evt->data, evt->size);

    return result;
}

void ui_sprite_event_handler_clear_all(ui_sprite_world_t world, ui_sprite_event_handler_list_t * manage) {
    while(!TAILQ_EMPTY(manage)) {
        ui_sprite_event_handler_free(world, TAILQ_FIRST(manage));
    }
}

uint32_t ui_sprite_event_handler_hash(ui_sprite_event_handler_t handler) {
    return (cpe_hash_str(handler->m_event_name, strlen(handler->m_event_name)) << 8)
        | (handler->m_event_entity & 0xFF);
}

int ui_sprite_event_handler_eq(ui_sprite_event_handler_t l, ui_sprite_event_handler_t r) {
    return l->m_event_entity == r->m_event_entity && strcmp(l->m_event_name, r->m_event_name) == 0;
}

