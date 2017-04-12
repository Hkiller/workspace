#include <assert.h>
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_tri_action_i.h"
#include "ui_sprite_tri_action_meta_i.h"

static ui_sprite_tri_action_t ui_sprite_tri_action_create_i(
    ui_sprite_tri_module_t module, ui_sprite_tri_rule_t rule, ui_sprite_tri_action_meta_t meta)
{
    ui_sprite_tri_action_t action;
    
    action = TAILQ_FIRST(&module->m_free_actions);
    if (action) {
        TAILQ_REMOVE(&module->m_free_actions, action, m_next);
    }
    else {
        action = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_tri_action) + module->m_action_max_data_capacity);
        if (action == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_tri_action_create: alloc action fail!");
            return NULL;
        }
    }

    action->m_rule = rule;
    action->m_meta = meta;
    action->m_trigger = ui_sprite_tri_action_in_active;
    TAILQ_INSERT_TAIL(&rule->m_actions, action, m_next);
    
    module->m_action_count++;
    return action;
}

static void ui_sprite_tri_action_free_i(ui_sprite_tri_module_t module, ui_sprite_tri_action_t action) {
    assert(action->m_rule);
    assert(action->m_meta);

    TAILQ_REMOVE(&action->m_rule->m_actions, action, m_next);
    
    assert(module->m_action_count > 0);
    module->m_action_count--;

    action->m_rule = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_actions, action, m_next);
}

ui_sprite_tri_action_t ui_sprite_tri_action_create(ui_sprite_tri_rule_t rule, const char * type_name) {
    ui_sprite_tri_module_t module = rule->m_obj->m_module;
    ui_sprite_tri_action_meta_t meta;
    ui_sprite_tri_action_t action;
    
    meta = ui_sprite_tri_action_meta_find(module, type_name);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tri_action_create: meta %s not exist!", type_name);
        return NULL;
    }

    action = ui_sprite_tri_action_create_i(module, rule, meta);
    if (action == NULL) return NULL;
    
    if (meta->m_init) {
        if (meta->m_init(meta->m_ctx, action) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_tri_action_create: init action fail!");
            ui_sprite_tri_action_free_i(module, action);
            return NULL;
        }
    }
    
    return action;
}

ui_sprite_tri_action_t ui_sprite_tri_action_clone(ui_sprite_tri_rule_t rule, ui_sprite_tri_action_t source) {
    ui_sprite_tri_module_t module = rule->m_obj->m_module;
    ui_sprite_tri_action_meta_t meta = source->m_meta;
    ui_sprite_tri_action_t action;
    
    action = ui_sprite_tri_action_create_i(module, rule, meta);
    if (action == NULL) return NULL;
    
    if (meta->m_copy) {
        if (meta->m_copy(meta->m_ctx, action, source) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_tri_action_clone: copy action fail!");
            ui_sprite_tri_action_free_i(module, action);
            return NULL;
        }
    }

    return action;
}

void ui_sprite_tri_action_free(ui_sprite_tri_action_t action) {
    ui_sprite_tri_module_t module = action->m_rule->m_obj->m_module;

    if (action->m_meta->m_fini) {
        action->m_meta->m_fini(action->m_meta->m_ctx, action);
    }

    ui_sprite_tri_action_free_i(module, action);
}

void ui_sprite_tri_action_real_free(ui_sprite_tri_action_t action) {
    ui_sprite_tri_module_t module = (void *)action->m_rule;

    TAILQ_REMOVE(&module->m_free_actions, action, m_next);

    mem_free(module->m_alloc, action);
}

ui_sprite_tri_action_trigger_t ui_sprite_tri_action_trigger(ui_sprite_tri_action_t action) {
    return action->m_trigger;
}

void ui_sprite_tri_action_set_trigger(ui_sprite_tri_action_t action, ui_sprite_tri_action_trigger_t trigger) {
    action->m_trigger = trigger;
}

ui_sprite_tri_action_meta_t ui_sprite_tri_action_meta(ui_sprite_tri_action_t action) {
    return action->m_meta;
}

ui_sprite_tri_rule_t ui_sprite_tri_action_rule(ui_sprite_tri_action_t action) {
    return action->m_rule;
}

ui_sprite_entity_t ui_sprite_tri_action_entity(ui_sprite_tri_action_t action) {
    return ui_sprite_component_entity(ui_sprite_component_from_data(action->m_rule->m_obj));
}

void * ui_sprite_tri_action_data(ui_sprite_tri_action_t action) {
    return action + 1;
}

ui_sprite_tri_action_t ui_sprite_tri_action_from_data(void * data) {
    return ((ui_sprite_tri_action_t)data) - 1;
}

void ui_sprite_tri_action_execute(ui_sprite_tri_action_t action) {
    action->m_meta->m_exec(action->m_meta->m_ctx, action);    
}
