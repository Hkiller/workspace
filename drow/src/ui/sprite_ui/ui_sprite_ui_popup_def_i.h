#ifndef UI_SPRITE_UI_POPUP_DEF_I_H
#define UI_SPRITE_UI_POPUP_DEF_I_H
#include "ui/sprite_cfg/ui_sprite_cfg_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_fsm.h"
#include "ui/sprite_ui/ui_sprite_ui_popup_def.h"
#include "ui_sprite_ui_env_i.h"

struct ui_sprite_ui_popup_def {
    ui_sprite_fsm_ins_t m_fsm;
};

int ui_sprite_ui_env_popup_def_init(void * ctx, plugin_ui_env_t b_env, plugin_ui_popup_def_t b_popup_def);
void ui_sprite_ui_env_popup_def_fini(void * ctx, plugin_ui_env_t b_env, plugin_ui_popup_def_t b_popup_def);
int ui_sprite_ui_env_popup_enter(void * ctx, plugin_ui_env_t b_env, plugin_ui_popup_t b_popup);
void ui_sprite_ui_env_popup_leave(void * ctx, plugin_ui_env_t b_env, plugin_ui_popup_t b_popup);

int ui_sprite_ui_env_popup_def_load(ui_sprite_ui_env_t env, plugin_ui_popup_def_t b_popup_def, ui_sprite_cfg_loader_t loader, cfg_t cfg);

#endif
