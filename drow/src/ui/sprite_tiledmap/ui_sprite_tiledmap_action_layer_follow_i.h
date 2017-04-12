#ifndef UI_SPRITE_SOUND_ACTION_LAYER_FOLLOW_I_H
#define UI_SPRITE_SOUND_ACTION_LAYER_FOLLOW_I_H
#include "ui/sprite_tiledmap/ui_sprite_tiledmap_action_layer_follow.h"
#include "ui_sprite_tiledmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tiledmap_action_layer_follow {
    ui_sprite_tiledmap_module_t m_module;
    struct ui_vector_2 m_cfg_lock_pos;
    uint8_t m_cfg_process_x;
    uint8_t m_cfg_process_y;
    char * m_cfg_follow_layer;
    char * m_cfg_control_layer;
    plugin_tiledmap_layer_t m_follow_layer;
    plugin_tiledmap_layer_t m_control_layer;
};

int ui_sprite_tiledmap_action_layer_follow_regist(ui_sprite_tiledmap_module_t module);
void ui_sprite_tiledmap_action_layer_follow_unregist(ui_sprite_tiledmap_module_t module);

#ifdef __cplusplus
}
#endif

#endif
