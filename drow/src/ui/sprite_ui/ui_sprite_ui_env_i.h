#ifndef UI_SPRITE_UI_ENV_I_H
#define UI_SPRITE_UI_ENV_I_H
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "ui/sprite_ui/ui_sprite_ui_env.h"
#include "ui_sprite_ui_module_i.h"

struct ui_sprite_ui_env {
    ui_sprite_ui_module_t m_module;
    plugin_ui_env_t m_env;
    ui_sprite_entity_t m_entity;
    struct plugin_ui_env_backend m_backend;
};

int ui_sprite_ui_env_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_env_unregist(ui_sprite_ui_module_t module);

ui_sprite_world_res_t ui_sprite_ui_env_res_load(void * ctx, ui_sprite_world_t world, cfg_t cfg);

/*for backend*/
void ui_sprite_ui_env_send_event(void * ctx, plugin_ui_env_t env, LPDRMETA meta, void * data, uint32_t data_size, dr_data_t overwrite);
void ui_sprite_ui_env_build_and_send_event(void * ctx, plugin_ui_env_t, const char * def, dr_data_source_t data_source, dr_data_t overwrite);

#endif
