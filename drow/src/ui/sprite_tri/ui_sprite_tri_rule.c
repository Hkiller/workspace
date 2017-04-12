#include <assert.h>
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_tri_rule_i.h"
#include "ui_sprite_tri_condition_i.h"
#include "ui_sprite_tri_action_i.h"
#include "ui_sprite_tri_trigger_i.h"

ui_sprite_tri_rule_t ui_sprite_tri_rule_create(ui_sprite_tri_obj_t obj) {
    ui_sprite_tri_module_t module = obj->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(obj);
    ui_sprite_tri_rule_t rule;

    rule = TAILQ_FIRST(&module->m_free_rules);
    if (rule) {
        TAILQ_REMOVE(&module->m_free_rules, rule, m_next);
    }
    else {
        rule = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_tri_rule));
        if (rule == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_tri_rule_create: alloc fail!");
            return NULL;
        }
    }

    rule->m_obj = obj;
    rule->m_condition = NULL;
    rule->m_active = 1;
    rule->m_effect = 0;
    TAILQ_INIT(&rule->m_actions);
    TAILQ_INIT(&rule->m_triggers);
    
    TAILQ_INSERT_TAIL(&obj->m_rules, rule, m_next);

    if (ui_sprite_component_is_active(component)) {
        ui_sprite_component_sync_update(component, 1);
    }

    return rule;
}

void ui_sprite_tri_rule_free(ui_sprite_tri_rule_t rule) {
    ui_sprite_tri_obj_t obj = rule->m_obj;
    ui_sprite_tri_module_t module = obj->m_module;

    if (rule->m_condition) {
        ui_sprite_tri_condition_free(rule->m_condition);
        assert(rule->m_condition == NULL);
    }

    while(!TAILQ_EMPTY(&rule->m_actions)) {
        ui_sprite_tri_action_free(TAILQ_FIRST(&rule->m_actions));
    }
    
    while(!TAILQ_EMPTY(&rule->m_triggers)) {
        ui_sprite_tri_trigger_free(TAILQ_FIRST(&rule->m_triggers));
    }
    
    TAILQ_REMOVE(&obj->m_rules, rule, m_next);

    rule->m_obj = (void*)obj->m_module;
    TAILQ_INSERT_TAIL(&module->m_free_rules, rule, m_next);
}

ui_sprite_tri_rule_t ui_sprite_tri_rule_clone(ui_sprite_tri_obj_t obj, ui_sprite_tri_rule_t source) {
    ui_sprite_tri_rule_t to_rule;
    ui_sprite_tri_action_t from_action;
    ui_sprite_tri_trigger_t from_trigger;
    
    to_rule = ui_sprite_tri_rule_create(obj);
    if (to_rule == NULL) return NULL;

    if (source->m_condition) {
        if (ui_sprite_tri_condition_clone(to_rule, source->m_condition) == NULL) {
            ui_sprite_tri_rule_free(to_rule);
            return NULL;
        }
    }

    TAILQ_FOREACH(from_action, &source->m_actions, m_next) {
        if (ui_sprite_tri_action_clone(to_rule, from_action) == NULL) {
            ui_sprite_tri_rule_free(to_rule);
            return NULL;
        }
    }

    TAILQ_FOREACH(from_trigger, &source->m_triggers, m_next) {
        if (ui_sprite_tri_trigger_clone(to_rule, from_trigger) == NULL) {
            ui_sprite_tri_rule_free(to_rule);
            return NULL;
        }
    }

    return to_rule;
}

uint8_t ui_sprite_tri_rule_is_active(ui_sprite_tri_rule_t rule) {
    return rule->m_active;
}

void ui_sprite_tri_rule_set_active(ui_sprite_tri_rule_t rule, uint8_t active) {
    rule->m_active = active;
}

uint8_t ui_sprite_tri_rule_is_effect(ui_sprite_tri_rule_t rule) {
    return rule->m_effect;
}

void ui_sprite_tri_rule_real_free(ui_sprite_tri_rule_t rule) {
    ui_sprite_tri_module_t module = (void*)rule->m_obj;

    TAILQ_REMOVE(&module->m_free_rules, rule, m_next);
    mem_free(module->m_alloc, rule);
}

void ui_sprite_tri_rule_sync_state(ui_sprite_tri_rule_t rule, uint8_t active) {
    ui_sprite_tri_action_t action;
    
    if (active) {
        if (!rule->m_effect) {
            rule->m_effect = 1;
            TAILQ_FOREACH(action, &rule->m_actions, m_next) {
                if (action->m_trigger == ui_sprite_tri_action_on_active || action->m_trigger == ui_sprite_tri_action_in_active) {
                    ui_sprite_tri_action_execute(action);
                }
            }
        }
        else {
            TAILQ_FOREACH(action, &rule->m_actions, m_next) {
                if (action->m_trigger == ui_sprite_tri_action_in_active) {
                    ui_sprite_tri_action_execute(action);
                }
            }
        }
    }
    else {
        if (rule->m_effect) {
            rule->m_effect = 0;
            TAILQ_FOREACH(action, &rule->m_actions, m_next) {
                if (action->m_trigger == ui_sprite_tri_action_on_deactive) {
                    ui_sprite_tri_action_execute(action);
                }
            }
        }
    }
}
