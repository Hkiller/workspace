#ifndef UI_SPRITE_BARRAGE_ENV_I_H
#define UI_SPRITE_BARRAGE_ENV_I_H
#include "plugin/barrage/plugin_barrage_env.h"
#include "ui/sprite_barrage/ui_sprite_barrage_env.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_barrage_env {
    ui_sprite_barrage_module_t m_module;
    plugin_barrage_env_t m_env;
};

int ui_sprite_barrage_env_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_env_unregist(ui_sprite_barrage_module_t module);

ui_sprite_world_res_t ui_sprite_barrage_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
