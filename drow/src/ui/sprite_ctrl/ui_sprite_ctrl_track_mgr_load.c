#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_ctrl_track_mgr_i.h"
#include "ui_sprite_ctrl_track_meta_i.h"
#include "ui_sprite_ctrl_module_i.h"

static int ui_sprite_cfg_load_resource_track_mgr_metas(ui_sprite_ctrl_module_t module, ui_sprite_ctrl_track_mgr_t track_mgr, cfg_t cfg);

ui_sprite_world_res_t ui_sprite_ctrl_track_mgr_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_track_mgr_t track_mgr;
    ui_sprite_ctrl_module_t ctrl_module;
    const char * ctrl_module_name;

    ctrl_module_name = cfg_get_string(cfg, "module", NULL);

    ctrl_module = ui_sprite_ctrl_module_find_nc(ui_sprite_world_app(world), ctrl_module_name);
    if (ctrl_module == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create track_mgr resource: ctrl module %s not exist!",
            ui_sprite_ctrl_module_name(module), ctrl_module_name ? ctrl_module_name : "default");
        return NULL;
    }

    track_mgr = ui_sprite_ctrl_track_mgr_create(ctrl_module, world);
    if (track_mgr == NULL) {
        CPE_ERROR(module->m_em, "%s: create track_mgr resource: create fail!", ui_sprite_ctrl_module_name(module));
        return NULL;
    }

    if (ui_sprite_cfg_load_resource_track_mgr_metas(module, track_mgr, cfg_find_cfg(cfg, "track-types")) != 0) {
        ui_sprite_ctrl_track_mgr_free(track_mgr);
        return NULL;
    }

    return ui_sprite_world_res_from_data(track_mgr);
}

static int ui_sprite_cfg_load_resource_track_mgr_metas(ui_sprite_ctrl_module_t module, ui_sprite_ctrl_track_mgr_t track_mgr, cfg_t cfg) {
    struct cfg_it meta_cfg_it;
    cfg_t meta_cfg;

    cfg_it_init(&meta_cfg_it, cfg);
    while((meta_cfg = cfg_it_next(&meta_cfg_it))) {
        const char * track_meta_name;
        ui_sprite_ctrl_track_meta_t track_meta;
        struct cfg_it point_cfg_it;
        cfg_t point_cfg;
        
        track_meta_name = cfg_name(meta_cfg);
        assert(track_meta_name);

        track_meta = ui_sprite_ctrl_track_meta_create(track_mgr, track_meta_name, cfg_get_string(meta_cfg, "anim-layer", ""));
        if (track_meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create track_mgr resource: track meta %s: create fail!",
                ui_sprite_ctrl_module_name(module), track_meta_name);
            return -1;
        }

        cfg_it_init(&point_cfg_it, cfg_find_cfg(meta_cfg, "points"));
        while((point_cfg = cfg_it_next(&point_cfg_it))) {
            const char * res;
            float interval;

            res = cfg_get_string(point_cfg, "res", NULL);
            if (res == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create track_mgr resource: track meta %s: point meta res not set!",
                    ui_sprite_ctrl_module_name(module), track_meta_name);
                return -1;
            }

            interval = cfg_get_float(point_cfg, "interval", 0.0f);

            if (ui_sprite_ctrl_track_meta_add_point(track_meta, interval, res) != 0) {
                CPE_ERROR(
                    module->m_em, "%s: create track_mgr resource: track meta %s: point meta res not set!",
                    ui_sprite_ctrl_module_name(module), track_meta_name);
                return -1;
            }
        }
    }

    return 0;
}
