#ifndef UI_SPRITE_UI_STATE_I_H
#define UI_SPRITE_UI_STATE_I_H
#include "plugin/ui/plugin_ui_state.h"
#include "ui/sprite_ui/ui_sprite_ui_state.h"
#include "ui_sprite_ui_phase_i.h"

struct ui_sprite_ui_state {
    ui_sprite_ui_env_t m_env;
    ui_sprite_fsm_state_t m_fsm_state;
    ui_sprite_fsm_state_t m_state_state;
};

int ui_sprite_ui_env_state_init(void * ctx, plugin_ui_state_t b_state);
void ui_sprite_ui_env_state_fini(void * ctx, plugin_ui_state_t b_state);

int ui_sprite_ui_env_state_node_active(void * ctx, plugin_ui_state_node_t state_node, plugin_ui_state_t state);
uint8_t ui_sprite_ui_env_state_node_is_active(void * ctx, plugin_ui_state_node_t state_node);
void ui_sprite_ui_env_state_node_deactive(void * ctx, plugin_ui_state_node_t state_node);

#endif
