#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_moving_env_i.h"

ui_sprite_world_res_t ui_sprite_moving_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_moving_module_t module = ctx;
    ui_sprite_moving_env_t env = ui_sprite_moving_env_create(module, world);

    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create moving_env resource: create moving_env fail!",
            ui_sprite_moving_module_name(module));
        return NULL;
    }

    return ui_sprite_world_res_from_data(env);
}
