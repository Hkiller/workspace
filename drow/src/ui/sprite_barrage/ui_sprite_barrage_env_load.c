#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/app_context.h"
#include "plugin/barrage/plugin_barrage_group.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_barrage_env_i.h"

ui_sprite_world_res_t ui_sprite_barrage_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_env_t env = ui_sprite_barrage_env_create(module, world);
    struct cfg_it group_it;
    cfg_t group_cft;

    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create barrage_env resource: create barrage_env fail!",
            ui_sprite_barrage_module_name(module));
        return NULL;
    }

    if (ui_sprite_barrage_env_set_update_priority(env, cfg_get_int8(cfg, "update-priority", 0)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create barrage_env resource: set update priority %d fail!",
            ui_sprite_barrage_module_name(module), cfg_get_int8(cfg, "update-priority", 0));
        ui_sprite_barrage_env_free(world);
        return NULL;
    }

    cfg_it_init(&group_it, cfg_find_cfg(cfg, "groups"));
    while((group_cft = cfg_it_next(&group_it))) {
        const char * group_name = cfg_as_string(group_cft, NULL);
        if (group_name == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create barrage_env resource: group config format error",
                ui_sprite_barrage_module_name(module));
            ui_sprite_barrage_env_free(world);
            return NULL;
        }

        if (plugin_barrage_group_create(env->m_env, group_name) == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create barrage_env resource: create group %s error",
                ui_sprite_barrage_module_name(module), group_name);
            ui_sprite_barrage_env_free(world);
            return NULL;
        }
    }
            
    return ui_sprite_world_res_from_data(env);
}
