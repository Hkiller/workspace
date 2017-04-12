#ifndef UI_SPRITE_UI_ACTION_ANIM_BULK_I_H
#define UI_SPRITE_UI_ACTION_ANIM_BULK_I_H
#include "plugin/ui/plugin_ui_animation.h"
#include "ui/sprite_ui/ui_sprite_ui_action_control_anim_bulk.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_ui_action_control_anim_bulk_record_list, ui_sprite_ui_action_control_anim_bulk_record) ui_sprite_ui_action_control_anim_bulk_record_list_t;

struct ui_sprite_ui_action_control_anim_bulk {
    ui_sprite_ui_module_t m_module;
    ui_sprite_ui_action_control_anim_bulk_record_list_t m_records;
    ui_sprite_ui_action_control_anim_bulk_record_list_t m_runing_records;
    ui_sprite_ui_action_control_anim_bulk_record_list_t m_waiting_records;
};

int ui_sprite_ui_action_control_anim_bulk_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_control_anim_bulk_unregist(ui_sprite_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
