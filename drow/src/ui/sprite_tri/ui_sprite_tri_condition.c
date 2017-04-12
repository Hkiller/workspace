#include <assert.h>
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_tri_condition_i.h"
#include "ui_sprite_tri_condition_meta_i.h"

static ui_sprite_tri_condition_t ui_sprite_tri_condition_create_i(
    ui_sprite_tri_module_t module, ui_sprite_tri_rule_t rule, ui_sprite_tri_condition_meta_t meta)
{
    ui_sprite_tri_condition_t condition;
    
    if (rule->m_condition != NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tri_condition_create: rule already have condition!");
        return NULL;
    }
    
    condition = TAILQ_FIRST(&module->m_free_conditions);
    if (condition) {
        TAILQ_REMOVE(&module->m_free_conditions, condition, m_next);
    }
    else {
        condition = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_tri_condition) + module->m_condition_max_data_capacity);
        if (condition == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_tri_condition_create: alloc condition fail!");
            return NULL;
        }
    }

    condition->m_rule = rule;
    condition->m_meta = meta;
    rule->m_condition = condition;
    module->m_condition_count++;
    return condition;
}

static void ui_sprite_tri_condition_free_i(ui_sprite_tri_module_t module, ui_sprite_tri_condition_t condition) {
    assert(condition->m_rule);
    assert(condition->m_rule->m_condition == condition);
    assert(condition->m_meta);
    
    condition->m_rule->m_condition = NULL;
    
    assert(module->m_condition_count > 0);
    module->m_condition_count--;

    condition->m_rule = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_conditions, condition, m_next);
}

ui_sprite_tri_condition_t ui_sprite_tri_condition_create(ui_sprite_tri_rule_t rule, const char * type_name) {
    ui_sprite_tri_module_t module = rule->m_obj->m_module;
    ui_sprite_tri_condition_meta_t meta;
    ui_sprite_tri_condition_t condition;
    
    meta = ui_sprite_tri_condition_meta_find(module, type_name);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tri_condition_create: meta %s not exist!", type_name);
        return NULL;
    }

    condition = ui_sprite_tri_condition_create_i(module, rule, meta);
    if (condition == NULL) return NULL;
    
    if (meta->m_init) {
        if (meta->m_init(meta->m_ctx, condition) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_tri_condition_create: init condition fail!");
            ui_sprite_tri_condition_free_i(module, condition);
            return NULL;
        }
    }
    
    return condition;
}

ui_sprite_tri_condition_t ui_sprite_tri_condition_clone(ui_sprite_tri_rule_t rule, ui_sprite_tri_condition_t source) {
    ui_sprite_tri_module_t module = rule->m_obj->m_module;
    ui_sprite_tri_condition_meta_t meta = source->m_meta;
    ui_sprite_tri_condition_t condition;
    
    condition = ui_sprite_tri_condition_create_i(module, rule, meta);
    if (condition == NULL) return NULL;
    
    if (meta->m_copy) {
        if (meta->m_copy(meta->m_ctx, condition, source) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_tri_condition_clone: copy condition fail!");
            ui_sprite_tri_condition_free_i(module, condition);
            return NULL;
        }
    }

    return condition;
}

void ui_sprite_tri_condition_free(ui_sprite_tri_condition_t condition) {
    ui_sprite_tri_module_t module = condition->m_rule->m_obj->m_module;

    if (condition->m_meta->m_fini) {
        condition->m_meta->m_fini(condition->m_meta->m_ctx, condition);
    }

    ui_sprite_tri_condition_free_i(module, condition);
}

void ui_sprite_tri_condition_real_free(ui_sprite_tri_condition_t condition) {
    ui_sprite_tri_module_t module = (void *)condition->m_rule;

    TAILQ_REMOVE(&module->m_free_conditions, condition, m_next);

    mem_free(module->m_alloc, condition);
}

ui_sprite_tri_condition_meta_t ui_sprite_tri_condition_meta(ui_sprite_tri_condition_t condition) {
    return condition->m_meta;
}

ui_sprite_tri_rule_t ui_sprite_tri_condition_rule(ui_sprite_tri_condition_t condition) {
    return condition->m_rule;
}

ui_sprite_entity_t ui_sprite_tri_condition_entity(ui_sprite_tri_condition_t condition) {
    return ui_sprite_component_entity(ui_sprite_component_from_data(condition->m_rule->m_obj));
}

void * ui_sprite_tri_condition_data(ui_sprite_tri_condition_t condition) {
    return condition + 1;
}

ui_sprite_tri_condition_t ui_sprite_tri_condition_from_data(void * data) {
    return ((ui_sprite_tri_condition_t)data) - 1;
}

uint8_t ui_sprite_tri_condition_check(ui_sprite_tri_condition_t condition, uint8_t * r) {
    return condition->m_meta->m_check(condition->m_meta->m_ctx, condition, r);
}
