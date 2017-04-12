#ifndef UI_SPRITE_UI_PHASE_I_H
#define UI_SPRITE_UI_PHASE_I_H
#include "plugin/ui/plugin_ui_phase.h"
#include "ui/sprite_cfg/ui_sprite_cfg_types.h"
#include "ui/sprite_ui/ui_sprite_ui_phase.h"
#include "ui_sprite_ui_env_i.h"

struct ui_sprite_ui_phase {
    ui_sprite_fsm_ins_t m_fsm;
    ui_sprite_fsm_state_t m_phase_state;
};

int ui_sprite_ui_env_phase_init(void * ctx, plugin_ui_env_t b_env, plugin_ui_phase_t b_phase);
void ui_sprite_ui_env_phase_fini(void * ctx, plugin_ui_env_t b_env, plugin_ui_phase_t b_phase);
int ui_sprite_ui_env_phase_enter(void * ctx, plugin_ui_env_t b_env, plugin_ui_phase_t b_phase);
void ui_sprite_ui_env_phase_leave(void * ctx, plugin_ui_env_t b_env, plugin_ui_phase_t b_phase);

int ui_sprite_ui_env_phase_load_without_navigations(ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, ui_sprite_cfg_loader_t loader, cfg_t cfg);
int ui_sprite_ui_env_phase_load_navigations(ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, cfg_t cfg);

#endif
