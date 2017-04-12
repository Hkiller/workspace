#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_event.h"
#include "ui_sprite_tri_trigger_i.h"
#include "ui_sprite_tri_condition_i.h"
#include "ui_sprite_tri_action_i.h"

static void ui_sprite_tri_trigger_on_event(void * ctx, ui_sprite_event_t evt);
static void ui_sprite_tri_trigger_on_attr(void * ctx);

static ui_sprite_tri_trigger_t ui_sprite_tri_trigger_create_i(
    ui_sprite_tri_module_t module, ui_sprite_tri_rule_t rule, ui_sprite_tri_trigger_type_t type);
static void ui_sprite_tri_trigger_free_i(ui_sprite_tri_module_t module, ui_sprite_tri_trigger_t trigger);

ui_sprite_tri_trigger_t ui_sprite_tri_trigger_create_on_event(ui_sprite_tri_rule_t rule, const char * event, const char * condition) {
    ui_sprite_tri_module_t module = rule->m_obj->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(rule->m_obj);
    ui_sprite_tri_trigger_t trigger;
    
    trigger = ui_sprite_tri_trigger_create_i(module, rule, ui_sprite_tri_trigger_type_on_event);
    if (trigger == NULL) return NULL;

    if (condition) {
        trigger->m_event.m_condition = cpe_str_mem_dup(module->m_alloc, condition);
        if (trigger->m_event.m_condition == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_tri_trigger_on_event: dup condition %s fail!", condition);
            ui_sprite_tri_trigger_free_i(module, trigger);
            return NULL;
        }
    }

    trigger->m_event.m_handler =
        ui_sprite_component_add_event_handler(
            component, ui_sprite_event_scope_self, event, 
            ui_sprite_tri_trigger_on_event, trigger);
    if (trigger->m_event.m_handler == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tri_trigger_on_event: create event handler fail!");
        ui_sprite_tri_trigger_free_i(module, trigger);
        return NULL;
    }
    
    return trigger;
}

ui_sprite_tri_trigger_t ui_sprite_tri_trigger_create_on_attr(ui_sprite_tri_rule_t rule, const char * condition) {
    ui_sprite_tri_module_t module = rule->m_obj->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(rule->m_obj);
    ui_sprite_tri_trigger_t trigger;

    assert(condition);
    
    trigger = ui_sprite_tri_trigger_create_i(module, rule, ui_sprite_tri_trigger_type_on_attr);
    if (trigger == NULL) return NULL;

    trigger->m_attr.m_condition = cpe_str_mem_dup(module->m_alloc, condition);
    if (trigger->m_attr.m_condition == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tri_trigger_on_event: dup condition %s fail!", condition);
        ui_sprite_tri_trigger_free_i(module, trigger);
        return NULL;
    }

    trigger->m_attr.m_monitor =
        ui_sprite_component_add_attr_monitor_by_def(
            component, condition, ui_sprite_tri_trigger_on_attr, trigger);
    if (trigger->m_attr.m_monitor == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tri_trigger_on_event: create attr monitor from def %s fail!", condition);
        ui_sprite_tri_trigger_free_i(module, trigger);
        return NULL;
    }
    
    return trigger;
}

ui_sprite_tri_trigger_t ui_sprite_tri_trigger_clone(ui_sprite_tri_rule_t rule, ui_sprite_tri_trigger_t source) {
    switch(source->m_type) {
    case ui_sprite_tri_trigger_type_on_event:
        return ui_sprite_tri_trigger_create_on_event(
            rule,
            ui_sprite_event_handler_process_event(source->m_event.m_handler),
            source->m_event.m_condition);
    case ui_sprite_tri_trigger_type_on_attr:
        return ui_sprite_tri_trigger_create_on_attr(rule, source->m_attr.m_condition);
    default:
        CPE_ERROR(rule->m_obj->m_module->m_em, "ui_sprite_tri_trigger_clone: unknown type %d!", source->m_type);
        return NULL;
    }
}

void ui_sprite_tri_trigger_free(ui_sprite_tri_trigger_t trigger) {
    ui_sprite_tri_module_t module = trigger->m_rule->m_obj->m_module;

    ui_sprite_tri_trigger_free_i(module, trigger);
}

void ui_sprite_tri_trigger_real_free(ui_sprite_tri_trigger_t trigger) {
    ui_sprite_tri_module_t module = (void *)trigger->m_rule;

    TAILQ_REMOVE(&module->m_free_triggers, trigger, m_next);

    mem_free(module->m_alloc, trigger);
}


static ui_sprite_tri_trigger_t ui_sprite_tri_trigger_create_i(
    ui_sprite_tri_module_t module, ui_sprite_tri_rule_t rule, ui_sprite_tri_trigger_type_t type)
{
    ui_sprite_tri_trigger_t trigger;
    
    trigger = TAILQ_FIRST(&module->m_free_triggers);
    if (trigger) {
        TAILQ_REMOVE(&module->m_free_triggers, trigger, m_next);
    }
    else {
        trigger = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_tri_trigger));
        if (trigger == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_tri_trigger_create: alloc trigger fail!");
            return NULL;
        }
    }

    bzero(trigger, sizeof(*trigger));
    
    trigger->m_rule = rule;
    trigger->m_type = type;
    TAILQ_INSERT_TAIL(&rule->m_triggers, trigger, m_next);

    return trigger;
}

static void ui_sprite_tri_trigger_free_i(ui_sprite_tri_module_t module, ui_sprite_tri_trigger_t trigger) {
    assert(trigger->m_rule);

    switch(trigger->m_type) {
    case ui_sprite_tri_trigger_type_on_event:
        if (trigger->m_event.m_handler) {
            ui_sprite_event_handler_free(
                ui_sprite_entity_world(
                    ui_sprite_component_entity(
                        ui_sprite_component_from_data(trigger->m_rule->m_obj))),
                trigger->m_event.m_handler);
            trigger->m_event.m_handler = NULL;
        }
        
        if (trigger->m_event.m_condition) {
            mem_free(module->m_alloc, trigger->m_event.m_condition);
            trigger->m_event.m_condition = NULL;
        }
        break;
    case ui_sprite_tri_trigger_type_on_attr:
        if (trigger->m_attr.m_monitor) {
            ui_sprite_attr_monitor_free(
                ui_sprite_entity_world(
                    ui_sprite_component_entity(
                        ui_sprite_component_from_data(trigger->m_rule->m_obj))),
                trigger->m_attr.m_monitor);
            trigger->m_attr.m_monitor = NULL;
        }
        
        if (trigger->m_attr.m_condition) {
            mem_free(module->m_alloc, trigger->m_attr.m_condition);
            trigger->m_attr.m_condition = NULL;
        }
        break;
    default:
        break;
    }
    
    TAILQ_REMOVE(&trigger->m_rule->m_triggers, trigger, m_next);
    
    trigger->m_rule = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_triggers, trigger, m_next);
}

static void ui_sprite_tri_trigger_on_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_tri_trigger_t triger = ctx;
    ui_sprite_tri_rule_t rule = triger->m_rule;
    ui_sprite_tri_module_t module = rule->m_obj->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(triger->m_rule->m_obj));

    //printf("xxxxx: on event\n");

    if (triger->m_event.m_condition) {
        uint8_t check_r;
        struct dr_data_source data_source;
        
        data_source.m_data.m_meta = evt->meta;
        data_source.m_data.m_data = (void*)evt->data;
        data_source.m_data.m_size = evt->size;
        data_source.m_next = NULL;

        if (ui_sprite_entity_try_calc_bool(&check_r, triger->m_attr.m_condition, entity, &data_source, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): tri on event %s: calc condition %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                dr_meta_name(evt->meta), triger->m_event.m_condition);
            return;
        }

        if (!check_r) return;
    }

    if (rule->m_condition) {
        uint8_t check_r;
        if (ui_sprite_tri_condition_check(rule->m_condition, &check_r) != 0) return;
        ui_sprite_tri_rule_sync_state(rule, check_r);
    }
    else {
        ui_sprite_tri_action_t action;

        TAILQ_FOREACH(action, &rule->m_actions, m_next) {
            if (action->m_trigger == ui_sprite_tri_action_on_active) {
                ui_sprite_tri_action_execute(action);
            }
        }
    }
}

static void ui_sprite_tri_trigger_on_attr(void * ctx) {
    ui_sprite_tri_trigger_t triger = ctx;
    ui_sprite_tri_rule_t rule = triger->m_rule;
    ui_sprite_tri_module_t module = rule->m_obj->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(triger->m_rule->m_obj));
    uint8_t check_r;

    if (!rule->m_active) return;
    
    if (ui_sprite_entity_try_calc_bool(&check_r, triger->m_attr.m_condition, entity, NULL, module->m_em) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tri on attr update: calc condition %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), triger->m_attr.m_condition);
        return;
    }

    if (check_r && rule->m_condition) {
        if (ui_sprite_tri_condition_check(rule->m_condition, &check_r) != 0) return;
    }
    
    ui_sprite_tri_rule_sync_state(rule, check_r);
}
