#ifndef UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_I_H
#define UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_I_H
#include "ui/sprite_ui/ui_sprite_ui_action_show_template.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_ui_action_show_template_binding * ui_sprite_ui_action_show_template_binding_t;
typedef TAILQ_HEAD(
    ui_sprite_ui_action_show_template_binding_list,
    ui_sprite_ui_action_show_template_binding) ui_sprite_ui_action_show_template_binding_list_t;
typedef struct ui_sprite_ui_action_show_template_def * ui_sprite_ui_action_show_template_def_t;
typedef TAILQ_HEAD(
    ui_sprite_ui_action_show_template_def_list,
    ui_sprite_ui_action_show_template_def) ui_sprite_ui_action_show_template_def_list_t;
    
struct ui_sprite_ui_action_show_template {
    ui_sprite_ui_module_t m_module;
    char * m_cfg_res;
    char * m_cfg_group;
    uint8_t m_cfg_is_free;
    ui_sprite_ui_action_show_template_def_list_t m_cfg_defs;
    
    uint32_t m_anim_id;
    ui_sprite_ui_action_show_template_binding_list_t m_bindings;
};

int ui_sprite_ui_action_show_template_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_show_template_unregist(ui_sprite_ui_module_t module);

struct ui_sprite_ui_action_show_template_def {
    const char * m_control;
    const char * m_attr_name;
    const char * m_attr_value;
    TAILQ_ENTRY(ui_sprite_ui_action_show_template_def) m_next;
};

ui_sprite_ui_action_show_template_def_t
ui_sprite_ui_action_show_template_def_create(
    ui_sprite_ui_action_show_template_t show_template, const char * ctrl_name, const char * attr_name, const char * attr_value);
void ui_sprite_ui_action_show_template_def_free(
    ui_sprite_ui_action_show_template_t show_template, ui_sprite_ui_action_show_template_def_t def);    
int ui_sprite_ui_action_show_template_def_set_value(
    ui_sprite_ui_action_show_template_def_t def, ui_sprite_ui_action_show_template_t show_template,
    ui_sprite_render_anim_t anim, ui_sprite_entity_t entity, ui_sprite_fsm_action_t action);

struct ui_sprite_ui_action_show_template_binding {
    const char * m_attr_name;
    ui_sprite_ui_action_show_template_t m_show_template;
    ui_sprite_ui_action_show_template_def_t m_def;
    TAILQ_ENTRY(ui_sprite_ui_action_show_template_binding) m_next;
};

ui_sprite_ui_action_show_template_binding_t
ui_sprite_ui_action_show_template_binding_create(
    ui_sprite_ui_action_show_template_t show_template, ui_sprite_ui_action_show_template_def_t def, const char * attr);

void ui_sprite_ui_action_show_template_binding_free(ui_sprite_ui_action_show_template_binding_t binding);

#ifdef __cplusplus
}
#endif

#endif
