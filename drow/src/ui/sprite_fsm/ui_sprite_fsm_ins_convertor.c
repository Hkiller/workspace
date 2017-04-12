#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_event.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui_sprite_fsm_ins_convertor_i.h"
#include "ui_sprite_fsm_ins_action_i.h"
#include "ui_sprite_fsm_ins_event_binding_i.h"
#include "ui_sprite_fsm_action_meta_i.h"

ui_sprite_fsm_convertor_t
ui_sprite_fsm_convertor_create(
    ui_sprite_fsm_action_t action,
    const char * event, const char * condition, const char * convert_to)
{
    ui_sprite_fsm_module_t module = action->m_state->m_ins->m_module;
    ui_sprite_fsm_convertor_t convertor;
    size_t event_name_len = strlen(event) + 1;
    size_t convert_to_len = strlen(convert_to) + 1;
    size_t condition_len = condition ? (strlen(condition) + 1) : 0;
                                               
    convertor = mem_alloc(
        module->m_alloc,
        sizeof(struct ui_sprite_fsm_convertor) + event_name_len + convert_to_len + condition_len);
    if (convertor == NULL) {
        CPE_ERROR(module->m_em, "action %s: create convertor: alloc fail!", action->m_name);
        return NULL;
    }

    convertor->m_action = action;
    convertor->m_handler = NULL;
    convertor->m_event = (const char *)(convertor + 1);
    convertor->m_convert_to = convertor->m_event + event_name_len;
    convertor->m_condition = condition ? (convertor->m_convert_to + convert_to_len) : NULL;

    memcpy((void*)convertor->m_event, event, event_name_len);
    memcpy((void*)convertor->m_convert_to, convert_to, convert_to_len);
    if (condition) {
        memcpy((void*)convertor->m_condition, condition, condition_len);
    }

    TAILQ_INSERT_TAIL(&action->m_convertors, convertor, m_next_for_action);

    return convertor;
}

ui_sprite_fsm_convertor_t
ui_sprite_fsm_convertor_clone(
    ui_sprite_fsm_action_t fsm_action, ui_sprite_fsm_convertor_t from)
{
    ui_sprite_fsm_convertor_t convertor = 
        ui_sprite_fsm_convertor_create(
            fsm_action,
            from->m_event, NULL, from->m_convert_to);

    return convertor;
}

void ui_sprite_fsm_convertor_free(ui_sprite_fsm_convertor_t convertor) {
    ui_sprite_fsm_action_t action = convertor->m_action;
    ui_sprite_fsm_module_t module = action->m_state->m_ins->m_module;

    if (convertor->m_handler) {
        ui_sprite_fsm_convertor_exit(convertor);
        assert(convertor->m_handler == NULL);
    }

    TAILQ_REMOVE(&action->m_convertors, convertor, m_next_for_action);

    mem_free(module->m_alloc, convertor);
}

static void ui_sprite_fsm_convertor_on_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_fsm_convertor_t convertor = ctx;
    ui_sprite_fsm_action_t fsm_action = convertor->m_action;
    ui_sprite_fsm_ins_t fsm = fsm_action->m_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_fsm_action_event_binding_t binding;
    ui_sprite_event_t convert_to;
    struct dr_data_source input_evt_data_source;
    const char * event_name;
    uint16_t process_count = 0;
    struct ui_sprite_fsm_addition_source_ctx addition_source_ctx;
    dr_data_source_t data_source;
    
    input_evt_data_source.m_data.m_meta = evt->meta;
    input_evt_data_source.m_data.m_data = (void*)evt->data;
    input_evt_data_source.m_data.m_size = evt->size;
    input_evt_data_source.m_next = NULL;

    data_source = &input_evt_data_source;
    
    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &addition_source_ctx);

    if (convertor->m_condition) {
        if (!ui_sprite_entity_calc_bool_with_dft(convertor->m_condition, entity, data_source, 0)) {
            if (ui_sprite_entity_debug(entity) >= 2) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): %s: action %s(%s): ignore convert event %s, condition[%s] check fail",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                    fsm_action->m_meta->m_name, fsm_action->m_name, dr_meta_name(evt->meta), convertor->m_condition);
            }
            return;
        }
    }

    fsm_action->m_addition_event = evt;

    convert_to = ui_sprite_fsm_action_build_event(convertor->m_action, module->m_alloc, convertor->m_convert_to, data_source);
    if (convert_to == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) build convert event %s ==> '%s' fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name, dr_meta_name(evt->meta), convertor->m_convert_to);
        fsm_action->m_addition_event = NULL;
        return;
    }
    convert_to->from_entity_id = evt->from_entity_id;

    event_name = dr_meta_name(convert_to->meta);

    TAILQ_FOREACH(binding, &fsm_action->m_event_bindings, m_next) {
        if (strcmp(ui_sprite_event_handler_process_event(binding->m_handler), event_name) == 0) {
            ++process_count;

            if (ui_sprite_entity_debug(entity)) {
                struct mem_buffer buffer;
                mem_buffer_init(&buffer, module->m_alloc);
                
                CPE_INFO(
                    module->m_em, "entity %d(%s): %s: action %s(%s) send convert event %s: %s",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                    fsm_action->m_meta->m_name, fsm_action->m_name, dr_meta_name(convert_to->meta),
                    dr_json_dump_inline(&buffer, convert_to->data, convert_to->size, convert_to->meta));

                mem_buffer_clear(&buffer);
            }

            (*ui_sprite_event_handler_process_fun(binding->m_handler))
                (ui_sprite_event_handler_process_ctx(binding->m_handler), convert_to);
        }
    }

    if (process_count == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) no handler process convert event %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name, dr_meta_name(convert_to->meta));
    }

    fsm_action->m_addition_event = NULL;

    mem_free(module->m_alloc, convert_to);
}

int ui_sprite_fsm_convertor_enter(ui_sprite_fsm_convertor_t convertor) {
    ui_sprite_fsm_action_t action = convertor->m_action;
    ui_sprite_fsm_module_t module = action->m_state->m_ins->m_module;
    ui_sprite_component_t component = ui_sprite_fsm_to_component(action->m_state->m_ins);
    ui_sprite_fsm_ins_t fsm = action->m_state->m_ins;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);

    if (convertor->m_handler) {
        CPE_ERROR(
            module->m_em, "entity %d(%s) %s: action %s(%s): convertor of %s enter: already active!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            action->m_meta->m_name, action->m_name, convertor->m_event);
        return -1;
    }

    convertor->m_handler =
        ui_sprite_component_add_event_handler(
            component,
            ui_sprite_event_scope_self,
            convertor->m_event,
            ui_sprite_fsm_convertor_on_event, convertor);
    if (convertor->m_handler == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s) %s: action %s(%s): convertor of %s enter: add event handler fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            action->m_meta->m_name, action->m_name, convertor->m_event);
        return -1;
    }

    return 0;
}

void ui_sprite_fsm_convertor_exit(ui_sprite_fsm_convertor_t convertor) {
    if (convertor->m_handler) {
        ui_sprite_event_handler_free(
            ui_sprite_fsm_to_world(convertor->m_action->m_state->m_ins),
            convertor->m_handler);
        convertor->m_handler = NULL;
    }
}

void ui_sprite_fsm_convertor_free_all(ui_sprite_fsm_action_t action) {
    while(!TAILQ_EMPTY(&action->m_convertors)) {
        ui_sprite_fsm_convertor_free(TAILQ_FIRST(&action->m_convertors));
    }
}

int ui_sprite_fsm_convertor_enter_all(ui_sprite_fsm_action_t action) {
    ui_sprite_fsm_convertor_t convertor;

    TAILQ_FOREACH(convertor, &action->m_convertors, m_next_for_action) {
        if (ui_sprite_fsm_convertor_enter(convertor) != 0) {
            ui_sprite_fsm_convertor_t fall_back;
            for(fall_back = TAILQ_FIRST(&action->m_convertors);
                fall_back != convertor;
                fall_back = TAILQ_NEXT(fall_back, m_next_for_action))
            {
                ui_sprite_fsm_convertor_exit(fall_back);
            }

            return -1;
        }
    }

    return 0;
}

void ui_sprite_fsm_convertor_exit_all(ui_sprite_fsm_action_t action) {
    ui_sprite_fsm_convertor_t convertor;

    TAILQ_FOREACH(convertor, &action->m_convertors, m_next_for_action) {
        ui_sprite_fsm_convertor_exit(convertor);
    }
}

