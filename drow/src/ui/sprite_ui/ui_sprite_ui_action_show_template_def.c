#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_template_render.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui_sprite_ui_action_show_template_i.h"

ui_sprite_ui_action_show_template_def_t
ui_sprite_ui_action_show_template_def_create(
    ui_sprite_ui_action_show_template_t show_template, const char * ctrl_name, const char * attr_name, const char * attr_value)
{
    ui_sprite_ui_module_t module = show_template->m_module;
    size_t ctrl_name_len = strlen(ctrl_name) + 1;
    size_t attr_name_len = strlen(attr_name) + 1;
    size_t attr_value_len = strlen(attr_value) + 1;
    char * p;
    ui_sprite_ui_action_show_template_def_t def;

    def = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_ui_action_show_template_def) + ctrl_name_len + attr_name_len + attr_value_len);
    if (def == NULL) {
        CPE_ERROR(
            module->m_em, "anim template %s: add def %s.%s = %s: alloc fail!",
            show_template->m_cfg_res, ctrl_name, attr_name, attr_value);
        return NULL;
    }

    p = (void*)(def + 1);

    cpe_str_dup_trim(p, ctrl_name_len, ctrl_name);
    def->m_control = p;
    p += ctrl_name_len;

    cpe_str_dup_trim(p, attr_name_len, attr_name);
    def->m_attr_name = p;
    p += attr_name_len;

    cpe_str_dup_trim(p, attr_value_len, attr_value);
    def->m_attr_value = p;
    p += attr_value_len;

    TAILQ_INSERT_TAIL(&show_template->m_cfg_defs, def, m_next);

    return def;
}

void ui_sprite_ui_action_show_template_def_free(ui_sprite_ui_action_show_template_t show_template, ui_sprite_ui_action_show_template_def_t def) {
    ui_sprite_ui_module_t module = show_template->m_module;

    TAILQ_REMOVE(&show_template->m_cfg_defs, def, m_next);

    mem_free(module->m_alloc, def);
}

static int ui_sprite_ui_action_show_template_set_value(
    ui_sprite_ui_module_t module, ui_sprite_render_anim_t anim, const char * ctrl_name, const char * attr_name, const char * value)
{
    ui_runtime_render_obj_t render_obj;
    plugin_ui_template_render_t template_render;
    plugin_ui_control_t control;
    
    render_obj = ui_runtime_render_obj_ref_obj(ui_sprite_render_anim_obj(anim));
    assert(render_obj);

    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_UI_TEMPLATE) {
        CPE_ERROR(
            module->m_em, "ui_sprite_render_stop_anim: anim %d is not template", ui_runtime_render_obj_type_id(render_obj));
        return -1;
    }

    template_render = ui_runtime_render_obj_data(render_obj);

    control = plugin_ui_template_render_control(template_render);
    assert(control);

    control = plugin_ui_control_find_child_by_path(control, ctrl_name);
    if (control == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_stop_anim: tempmlate can`t find control %s", ctrl_name);
        return -1;
    }

    if (plugin_ui_control_set_attr_by_str(control, attr_name, value) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_render_stop_anim: tempmlate control %s set %s value %s fail", ctrl_name, attr_name, value);
        return -1;
    }

    return 0;
}

int ui_sprite_ui_action_show_template_def_set_value(
    ui_sprite_ui_action_show_template_def_t def, ui_sprite_ui_action_show_template_t show_template,
    ui_sprite_render_anim_t anim, ui_sprite_entity_t entity, ui_sprite_fsm_action_t action)
{
    ui_sprite_ui_module_t module = show_template->m_module;
    const char * attr_value;

    attr_value = ui_sprite_fsm_action_try_calc_str(&module->m_dump_buffer, def->m_attr_value, action, NULL, module->m_em);
    if (attr_value == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): template %s: attr %s.%s: calc %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res,
            def->m_control, def->m_attr_name, def->m_attr_value);
        return -1;
    }

    if (ui_sprite_ui_action_show_template_set_value(module, anim, def->m_control, def->m_attr_name, attr_value) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): template %s: attr %s.%s: set value %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res,
            def->m_control, def->m_attr_name, attr_value);
        return -1;
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): template %s: attr %s.%s ==> %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res,
            def->m_control, def->m_attr_name, attr_value);
    }

    return 0;
}
