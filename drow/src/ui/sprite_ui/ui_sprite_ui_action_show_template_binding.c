#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui_sprite_ui_action_show_template_i.h"

static void ui_sprite_ui_action_show_template_binding_on_attr_updated(void * ctx);

ui_sprite_ui_action_show_template_binding_t
ui_sprite_ui_action_show_template_binding_create(
    ui_sprite_ui_action_show_template_t show_template, ui_sprite_ui_action_show_template_def_t def, const char * attr_name) {
    ui_sprite_ui_module_t module = show_template->m_module;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(show_template);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ui_action_show_template_binding_t show_template_binding;
    size_t attr_name_len = strlen(attr_name) + 1;

    show_template_binding = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_ui_action_show_template_binding) + attr_name_len);
    if (show_template_binding == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_action_show_template: start template %s: create binding %s.%s: alloc fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res,
            def->m_control, def->m_attr_name);
        return NULL;
    }

    if (ui_sprite_fsm_action_add_attr_monitor(
            fsm_action,
            attr_name, ui_sprite_ui_action_show_template_binding_on_attr_updated, show_template_binding)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_action_show_template: start template %s: create binding %s.%s: add monitor fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res,
            def->m_control, attr_name);
        mem_free(module->m_alloc, show_template_binding);
        return NULL;
    }

    show_template_binding->m_show_template = show_template;
    show_template_binding->m_def = def;
    show_template_binding->m_attr_name = (char*)(show_template_binding + 1);
    memcpy(show_template_binding + 1, attr_name, attr_name_len);

    TAILQ_INSERT_TAIL(&show_template->m_bindings, show_template_binding, m_next);

    return show_template_binding;
}

void ui_sprite_ui_action_show_template_binding_free(ui_sprite_ui_action_show_template_binding_t binding) {
    ui_sprite_ui_action_show_template_t show_template = binding->m_show_template;
    ui_sprite_ui_module_t module = show_template->m_module;

    TAILQ_REMOVE(&show_template->m_bindings, binding, m_next);

    mem_free(module->m_alloc, binding);
}

static void ui_sprite_ui_action_show_template_binding_on_attr_updated(void * ctx) {
    ui_sprite_ui_action_show_template_binding_t binding = ctx;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(binding->m_show_template);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_render_sch_t render_sch = ui_sprite_render_sch_find(entity);
    ui_sprite_render_anim_t anim;
    ui_sprite_ui_module_t module = binding->m_show_template->m_module;
    
    if (render_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_action_show_template: template %s: no render sch",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), binding->m_show_template->m_cfg_res);
        return;
    }

    anim = ui_sprite_render_anim_find_by_id(render_sch, binding->m_show_template->m_anim_id);
    if (anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_action_show_template: template %s: anim not exist ",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), binding->m_show_template->m_cfg_res);
        return;
    }
        
    if (ui_sprite_ui_action_show_template_def_set_value(binding->m_def, binding->m_show_template, anim, entity, fsm_action) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_action_show_template: template %s: set attr %s.%s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), binding->m_show_template->m_cfg_res,
            binding->m_def->m_control, binding->m_def->m_attr_name);
    }
}
