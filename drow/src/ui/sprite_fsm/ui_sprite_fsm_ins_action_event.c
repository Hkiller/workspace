#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/timer/timer_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data_entry.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_event.h"
#include "ui_sprite_fsm_ins_action_i.h"
#include "ui_sprite_fsm_ins_event_binding_i.h"
#include "ui_sprite_fsm_ins_attr_binding_i.h"
#include "ui_sprite_fsm_action_meta_i.h"
#include "ui_sprite_fsm_action_fsm_i.h"

void ui_sprite_fsm_action_append_addition_source(
    ui_sprite_fsm_action_t fsm_action, 
    dr_data_source_t * data_source,
    struct ui_sprite_fsm_addition_source_ctx * ctx)
{
    ui_sprite_fsm_state_t fsm_state = fsm_action->m_state;
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;
    uint8_t append_source_pos = 0;
    dr_data_source_t * insert_pos;

    ctx->m_last_source = data_source;
    while(*ctx->m_last_source) {
        ctx->m_last_source = &(*ctx->m_last_source)->m_next;
    }
    insert_pos = ctx->m_last_source;

    if (fsm_action->m_addition_event) {
        /*append addition event data*/
        ctx->m_append_sources[append_source_pos].m_data.m_meta = fsm_action->m_addition_event->meta;
        ctx->m_append_sources[append_source_pos].m_data.m_data = (void*)fsm_action->m_addition_event->data;
        ctx->m_append_sources[append_source_pos].m_data.m_size = fsm_action->m_addition_event->size;
        ctx->m_append_sources[append_source_pos].m_next = NULL;

        *insert_pos = &ctx->m_append_sources[append_source_pos++];
        insert_pos = &((*insert_pos)->m_next);
    }

    /*数据只有当前action会有 */
    if (fsm_action->m_meta->m_data_meta) {
        /*append action data*/
        ctx->m_append_sources[append_source_pos].m_data.m_meta = fsm_action->m_meta->m_data_meta;
        ctx->m_append_sources[append_source_pos].m_data.m_data = ((char *)ui_sprite_fsm_action_data(fsm_action)) + fsm_action->m_meta->m_data_start;
        ctx->m_append_sources[append_source_pos].m_data.m_size = fsm_action->m_meta->m_data_size;
        ctx->m_append_sources[append_source_pos].m_next = NULL;

        *insert_pos = &ctx->m_append_sources[append_source_pos++];
        insert_pos = &((*insert_pos)->m_next);
    }
        
    /*处理所有action路径上的事件 */
    while(fsm_state) {
        if (fsm_state->m_enter_event) {
            if (append_source_pos + 2 >= CPE_ARRAY_SIZE(ctx->m_append_sources)) break;
            
            /*append enter_evt common*/
            ctx->m_enter_event.enter_evt_from = fsm_state->m_enter_event->from_entity_id;
            cpe_str_dup(ctx->m_enter_event.enter_evt_name, sizeof(ctx->m_enter_event.enter_evt_name), dr_meta_name(fsm_state->m_enter_event->meta));

            ctx->m_append_sources[append_source_pos].m_data.m_meta = module->m_meta_action_enter_event;
            ctx->m_append_sources[append_source_pos].m_data.m_data = &ctx->m_enter_event;
            ctx->m_append_sources[append_source_pos].m_data.m_size = sizeof(ctx->m_enter_event);
            ctx->m_append_sources[append_source_pos].m_next = NULL;

            *insert_pos = &ctx->m_append_sources[append_source_pos++];
            insert_pos = &((*insert_pos)->m_next);

            /*append enter_evt data*/
            ctx->m_append_sources[append_source_pos].m_data.m_meta = fsm_state->m_enter_event->meta;
            ctx->m_append_sources[append_source_pos].m_data.m_data = (void*)fsm_state->m_enter_event->data;
            ctx->m_append_sources[append_source_pos].m_data.m_size = fsm_state->m_enter_event->size;
            ctx->m_append_sources[append_source_pos].m_next = NULL;

            *insert_pos = &ctx->m_append_sources[append_source_pos++];
            insert_pos = &((*insert_pos)->m_next);
        }

        if (fsm_state->m_ins->m_parent) {
            ui_sprite_fsm_action_fsm_t action_fsm = (ui_sprite_fsm_action_fsm_t)fsm_state->m_ins;
            if (action_fsm->m_data.m_meta) {
                if (append_source_pos + 1 >= CPE_ARRAY_SIZE(ctx->m_append_sources)) break;
                
                /*append enter_evt data*/
                ctx->m_append_sources[append_source_pos].m_data = action_fsm->m_data;
                ctx->m_append_sources[append_source_pos].m_next = NULL;

                *insert_pos = &ctx->m_append_sources[append_source_pos++];
                insert_pos = &((*insert_pos)->m_next);
            }
            
            fsm_state = fsm_state->m_ins->m_parent->m_cur_state;
        }
        else {
            break;
        }
    }
}

void ui_sprite_fsm_action_build_and_send_event(
    ui_sprite_fsm_action_t fsm_action,
    const char * event_def, dr_data_source_t data_source)
{
    struct ui_sprite_fsm_addition_source_ctx ctx;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    ui_sprite_component_build_and_send_event(
        ui_sprite_fsm_action_to_component(fsm_action), event_def, data_source);

    *ctx.m_last_source = NULL;
}

ui_sprite_event_t ui_sprite_fsm_action_build_event(
    ui_sprite_fsm_action_t op_action, mem_allocrator_t alloc, const char * def, dr_data_source_t data_source)
{
    struct ui_sprite_fsm_addition_source_ctx ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(op_action);
    ui_sprite_event_t r;
    
    ui_sprite_fsm_action_append_addition_source(op_action, &data_source, &ctx);

    r = ui_sprite_entity_build_event(entity, alloc, def, data_source);

    *ctx.m_last_source = NULL;

    return r;
}

static int ui_sprite_fsm_action_set_attr_one(
    ui_sprite_fsm_module_t module, ui_sprite_fsm_action_t fsm_action, ui_sprite_entity_t entity,
    char * def, dr_data_source_t data_source, dr_data_source_t input_data_sourc)
{
    dr_data_source_t check_source;
    struct dr_data_entry entry_buf;
    dr_data_entry_t attr = NULL;
    char * p = strchr(def, '=');
    char * arg_name = def;
    char * arg_value;
    uint8_t is_entity_attr = 0;
    
    if (p == NULL) {
        char * t = cpe_str_mem_dup(module->m_alloc, def);
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s): set attr one: def %s format error!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm_action->m_state->m_ins),
            fsm_action->m_meta->m_name, fsm_action->m_name, t);
        mem_free(module->m_alloc, t);
        return -1;
    }

    *((char*)cpe_str_trim_tail(p, arg_name)) = 0;
    arg_value = (char*)cpe_str_trim_head(p + 1);

    /*首先从数据源搜索目标数据，忽略输入数据 */
    for(check_source = data_source;
        check_source != NULL && check_source != input_data_sourc;
        check_source = check_source->m_next)
    {
        attr = dr_data_entry_find(&entry_buf, &check_source->m_data, arg_name);
        if (attr) break;
    }

    /*没有找到，则从Entity上搜索 */
    if (attr == NULL) {
        attr = ui_sprite_entity_find_attr(&entry_buf, entity, arg_name);
        if (attr == NULL) {
            char * t = cpe_str_mem_dup(module->m_alloc, arg_name);
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s): arg %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_fsm_ins_path(fsm_action->m_state->m_ins),
                fsm_action->m_meta->m_name, fsm_action->m_name, t);
            mem_free(module->m_alloc, t);
            return -1;
        }
        is_entity_attr = 1;
    }
    
    if (ui_sprite_data_build(attr, arg_value, ui_sprite_entity_world(entity), entity, data_source) != 0) {
        char * t = cpe_str_mem_dup(module->m_alloc, arg_name);
        char * t2 = cpe_str_mem_dup(module->m_alloc, arg_value);
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s): set attr %s ==> %s !",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm_action->m_state->m_ins),
            fsm_action->m_meta->m_name, fsm_action->m_name, t, t2);
        mem_free(module->m_alloc, t);
        mem_free(module->m_alloc, t2);
        return -1;
    }

    if (is_entity_attr) ui_sprite_entity_notify_attr_updated_one(entity, arg_name);

    if (ui_sprite_entity_debug(entity)) {
        struct mem_buffer value_buffer;
        char * t;
        const char * value;
        
        t = cpe_str_mem_dup(module->m_alloc, arg_name);
        mem_buffer_init(&value_buffer, module->m_alloc);

        if (dr_entry_ref_meta(attr->m_entry)) {
            value = dr_json_dump_inline(&value_buffer, attr->m_data, attr->m_size, dr_entry_ref_meta(attr->m_entry));
        }
        else {
            value = dr_ctype_to_string(&value_buffer, attr->m_data, dr_entry_type(attr->m_entry));
        }
                
        CPE_INFO(
            module->m_em, "entity %d(%s): %s: action %s(%s): set attr %s ==> %s ok!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm_action->m_state->m_ins),
            fsm_action->m_meta->m_name, fsm_action->m_name, t, value);
            
        mem_buffer_clear(&value_buffer);
        mem_free(module->m_alloc, t);
    }
    
    return 0;
}

int ui_sprite_fsm_action_bulk_set_attrs(
    ui_sprite_fsm_action_t fsm_action, const char * defs, dr_data_source_t data_source)
{
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;
    struct ui_sprite_fsm_addition_source_ctx ctx;
    dr_data_source_t input_data_sourc = data_source;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    char * allocked_tmp_args;
    char * tmp_args;
    char * p;

    allocked_tmp_args = cpe_str_mem_dup(module->m_alloc, defs);
    if (allocked_tmp_args == NULL) return -1;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);

    tmp_args = allocked_tmp_args;
    while((p = (char*)cpe_str_char_not_in_pair(tmp_args, ',', "{[(", ")]}"))) {
        char * arg_begin = tmp_args;
        char * arg_end = (char*)cpe_str_trim_tail(p, arg_begin);

        *arg_end = 0;
        tmp_args = (char*)cpe_str_trim_head(p + 1);

        if (ui_sprite_fsm_action_set_attr_one(module, fsm_action, entity, arg_begin, data_source, input_data_sourc) != 0) {
            mem_free(module->m_alloc, allocked_tmp_args);
            *ctx.m_last_source = NULL;
            return -1;
        }
    }

    if (*tmp_args != 0) {
        *(char *)cpe_str_trim_tail(tmp_args + strlen(tmp_args), tmp_args) = 0;

        if (ui_sprite_fsm_action_set_attr_one(module, fsm_action, entity, tmp_args, data_source, input_data_sourc) != 0) {
            mem_free(module->m_alloc, allocked_tmp_args);
            *ctx.m_last_source = NULL;
            return -1;
        }
    }

    mem_free(module->m_alloc, allocked_tmp_args);
    *ctx.m_last_source = NULL;

    return 0;
}

int ui_sprite_fsm_action_set_attr(
    ui_sprite_fsm_action_t fsm_action,
    const char * path, const char * value, dr_data_source_t data_source)
{
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;
    struct ui_sprite_fsm_addition_source_ctx ctx;
    dr_data_source_t input_data_sourc = data_source;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    char * tmp_args;

    mem_buffer_clear_data(&module->m_dump_buffer);
    mem_buffer_strcat(&module->m_dump_buffer, path);
    mem_buffer_strcat(&module->m_dump_buffer, "=");
    mem_buffer_strcat(&module->m_dump_buffer, value);
    
    tmp_args = mem_buffer_make_continuous(&module->m_dump_buffer, 0);
    if (tmp_args == NULL) return -1;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);

    *(char *)cpe_str_trim_tail(tmp_args + strlen(tmp_args), tmp_args) = 0;

    if (ui_sprite_fsm_action_set_attr_one(module, fsm_action, entity, tmp_args, data_source, input_data_sourc) != 0) {
        mem_buffer_clear_data(&module->m_dump_buffer);
        *ctx.m_last_source = NULL;
        return -1;
    }

    mem_buffer_clear_data(&module->m_dump_buffer);
    *ctx.m_last_source = NULL;

    return 0;
}

int ui_sprite_fsm_action_add_event_handler(
    ui_sprite_fsm_action_t fsm_action, ui_sprite_event_scope_t scope,
    const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx)
{
    ui_sprite_fsm_action_event_binding_t binding;
    ui_sprite_event_handler_t handler;
    ui_sprite_component_t component = ui_sprite_fsm_action_to_component(fsm_action);

    handler = ui_sprite_component_add_event_handler(component, scope, event_name, fun, ctx);
    if (handler == NULL) return -1;

    binding = ui_sprite_fsm_action_event_binding_create(fsm_action, handler);
    if (binding == NULL) {
        ui_sprite_event_handler_free(ui_sprite_fsm_action_to_world(fsm_action), handler);
        return -1;
    }

    return 0;
}

int ui_sprite_fsm_action_add_attr_monitor(
    ui_sprite_fsm_action_t fsm_action,
    const char * attrs, ui_sprite_attr_monitor_fun_t fun, void * ctx)
{
    ui_sprite_fsm_action_attr_binding_t binding;
    ui_sprite_attr_monitor_t handler;
    ui_sprite_component_t component = ui_sprite_fsm_action_to_component(fsm_action);

    handler = ui_sprite_component_add_attr_monitor(component, attrs, fun, ctx);
    if (handler == NULL) return -1;

    binding = ui_sprite_fsm_action_attr_binding_create(fsm_action, handler);
    if (binding == NULL) {
        ui_sprite_attr_monitor_free(ui_sprite_fsm_action_to_world(fsm_action), handler);
        return -1;
    }

    return 0;
}

int ui_sprite_fsm_action_add_attr_monitor_by_def(
    ui_sprite_fsm_action_t fsm_action,
    const char * def, ui_sprite_attr_monitor_fun_t fun, void * ctx)
{
    ui_sprite_fsm_action_attr_binding_t binding;
    ui_sprite_attr_monitor_t handler;
    ui_sprite_component_t component = ui_sprite_fsm_action_to_component(fsm_action);

    handler = ui_sprite_component_add_attr_monitor_by_def(component, def, fun, ctx);
    if (handler == NULL) return -1;

    binding = ui_sprite_fsm_action_attr_binding_create(fsm_action, handler);
    if (binding == NULL) {
        ui_sprite_attr_monitor_free(ui_sprite_fsm_action_to_world(fsm_action), handler);
        return -1;
    }

    return 0;
}

int ui_sprite_fsm_action_add_attr_monitor_by_defs(
    ui_sprite_fsm_action_t fsm_action,
    const char * * defs, uint16_t def_count, ui_sprite_attr_monitor_fun_t fun, void * ctx)
{
    ui_sprite_fsm_action_attr_binding_t binding;
    ui_sprite_attr_monitor_t handler;
    ui_sprite_component_t component = ui_sprite_fsm_action_to_component(fsm_action);

    handler = ui_sprite_component_add_attr_monitor_by_defs(component, defs, def_count, fun, ctx);
    if (handler == NULL) return -1;

    binding = ui_sprite_fsm_action_attr_binding_create(fsm_action, handler);
    if (binding == NULL) {
        ui_sprite_attr_monitor_free(ui_sprite_fsm_action_to_world(fsm_action), handler);
        return -1;
    }

    return 0;
}
