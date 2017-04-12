#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui_sprite_touch_env_i.h"

ui_sprite_world_res_t ui_sprite_touch_env_res_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_touch_mgr_t module = ctx;
    ui_sprite_touch_env_t env = ui_sprite_touch_env_create(module, world);

    if (env == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_touch_env_res_load: create env fail!");
        return NULL;
    }
    
    return ui_sprite_world_res_from_data(env);
}
