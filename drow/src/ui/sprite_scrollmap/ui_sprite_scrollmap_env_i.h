#ifndef UI_SPRITE_SCROLLMAP_ENV_I_H
#define UI_SPRITE_SCROLLMAP_ENV_I_H
#include "ui/sprite_scrollmap/ui_sprite_scrollmap_env.h"
#include "ui_sprite_scrollmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_scrollmap_env {
    ui_sprite_scrollmap_module_t m_module;
    ui_sprite_scrollmap_runtime_size_policy_t m_runtime_size_policy;
    uint8_t m_is_init;
    uint8_t m_debug;
    uint32_t m_entity_index;
    plugin_scrollmap_env_t m_env;
};

int ui_sprite_scrollmap_env_regist(ui_sprite_scrollmap_module_t module);
void ui_sprite_scrollmap_env_unregist(ui_sprite_scrollmap_module_t module);

ui_sprite_world_res_t ui_sprite_scrollmap_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
