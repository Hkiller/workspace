#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_world_res_t ui_sprite_ui_env_res_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_ui_module_t module = (ui_sprite_ui_module_t)ctx;
    ui_sprite_ui_env_t env = ui_sprite_ui_env_create(module, world);

    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create ui_env resource: create ui_env fail!",
            ui_sprite_ui_module_name(module));
        return NULL;
    }

    ui_sprite_ui_env_set_debug(env, cfg_get_uint8(cfg, "debug", 0));

    return ui_sprite_world_res_from_data(env);
}
