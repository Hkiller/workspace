#ifndef UI_SPRITE_UI_ACTION_SCOPE_VALUE_I_H
#define UI_SPRITE_UI_ACTION_SCOPE_VALUE_I_H
#include "ui/sprite_ui/ui_sprite_ui_action_scope_value.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_ui_action_scope_value_item * ui_sprite_ui_action_scope_value_item_t;
typedef TAILQ_HEAD(ui_sprite_ui_action_scope_value_item_list, ui_sprite_ui_action_scope_value_item) ui_sprite_ui_action_scope_value_item_list_t;

struct ui_sprite_ui_action_scope_value {
    ui_sprite_ui_module_t m_module;
    char * m_cfg_control;
    ui_sprite_ui_action_scope_value_item_list_t m_items;
};

struct ui_sprite_ui_action_scope_value_item {
    TAILQ_ENTRY(ui_sprite_ui_action_scope_value_item) m_next;
    char * m_cfg_name;
    char * m_cfg_value;
};

int ui_sprite_ui_action_scope_value_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_scope_value_unregist(ui_sprite_ui_module_t module);

ui_sprite_ui_action_scope_value_item_t
ui_sprite_ui_action_scope_value_item_create(
    ui_sprite_ui_action_scope_value_t scope_value, const char * cfg_name, const char * cfg_value);

void ui_sprite_ui_action_scope_value_item_free(
    ui_sprite_ui_action_scope_value_t scope_value, ui_sprite_ui_action_scope_value_item_t item);

#ifdef __cplusplus
}
#endif

#endif
