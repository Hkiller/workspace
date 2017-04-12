#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui/sprite_render/ui_sprite_render_layer.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui_sprite_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static int ui_sprite_chipmunk_env_create_debug_render(ui_sprite_chipmunk_env_t env, ui_sprite_world_t world, const char * layer_name);
    
ui_sprite_world_res_t ui_sprite_chipmunk_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env = ui_sprite_chipmunk_env_create(module, world);
    struct cfg_it mask_cfg_it;
    cfg_t layer_cfg;
    cfg_t spatial_hash_cfg;
    const char * value;
    float ptm;

    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create chipmunk_env resource: create chipmunk_env fail!",
            ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    env->m_debug = cfg_get_uint8(cfg, "debug", 0);

    if (ui_sprite_chipmunk_env_set_update_priority(env, cfg_get_int8(cfg, "update-priority", 0)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create chipmunk_env resource: set update priority %d fail!",
            ui_sprite_chipmunk_module_name(module), cfg_get_int8(cfg, "update-priority", 0));
        ui_sprite_chipmunk_env_free(world);
        return NULL;
    }

    ptm = cfg_get_float(cfg, "ptm-ratio", plugin_chipmunk_env_ptm(env->m_env));

    plugin_chipmunk_env_set_ptm(env->m_env, ptm);

    plugin_chipmunk_env_set_gravity(
        env->m_env, cfg_get_float(cfg, "gravity.x", 0.0f), cfg_get_float(cfg, "gravity.y", 0.0f) );

    plugin_chipmunk_env_set_step_duration(
        env->m_env, cfg_get_float(cfg, "step-duration", plugin_chipmunk_env_step_duration(env->m_env)));

    if ((spatial_hash_cfg = cfg_find_cfg(cfg, "spatial-hash"))) {
        float dim = cfg_get_float(spatial_hash_cfg, "dim", 0.0f);
        uint32_t count = cfg_get_uint32(spatial_hash_cfg, "count", 0);

        if (dim == 0.0f || count == 0) {
            CPE_ERROR(
                module->m_em, "%s: create chipmunk_env resource: spatial-hash: dim=%f, count=%d error!",
                ui_sprite_chipmunk_module_name(module), dim, count);
            ui_sprite_chipmunk_env_free(world);
            return NULL;
        }

        plugin_chipmunk_env_set_spatial_hash(env->m_env, dim, count);
    }
    
    /* Cpe::Cfg::Node const & boundary = cfg["boundary"]; */
    /* if (boundary.isValid()) { */
    /*     P2D::Pair lt; */
    /*     P2D::Pair br; */
    /*     lt.x = boundary["lt.x"].dft(0.0f); */
    /*     lt.y = boundary["lt.y"].dft(0.0f); */
    /*     br.x = boundary["br.x"].dft(960.0f); */
    /*     br.y = boundary["br.y"].dft(640.0f); */

    /*     world.setBoundary(lt, br); */
    /* } */

    cfg_it_init(&mask_cfg_it, cfg_find_cfg(cfg, "mask"));
    while((layer_cfg = cfg_it_next(&mask_cfg_it))) {
        const char * layer_name = cfg_as_string(layer_cfg, NULL);

        if (layer_name == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_env: load mask: layer name not configured");
            ui_sprite_chipmunk_env_free(world);
            return NULL;
        }

        if (layer_name[0] == 0) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_env: load mask: layer name empty");
            ui_sprite_chipmunk_env_free(world);
            return NULL;
        }

        if (plugin_chipmunk_env_mask_register(env->m_env, layer_name) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_env: load mask: register layer %s fail!", layer_name);
            ui_sprite_chipmunk_env_free(world);
            return NULL;
        }
    }

    if ((value = cfg_get_string(cfg, "ground", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &env->m_ground_mask, value) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_env: load ground: ground load mas %s!", value);
            ui_sprite_chipmunk_env_free(world);
            return NULL;
        }
    }

    if ((value = cfg_get_string(cfg, "debug-layer", NULL))) {
        if (ui_sprite_chipmunk_env_create_debug_render(env, world, value) != 0) {
            ui_sprite_chipmunk_env_free(world);
            return NULL;
        }
    }

    if (ui_sprite_chipmunk_env_set_process_touch(env, cfg_get_uint8(cfg, "touch", env->m_process_touch)) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_env: load touch: set process touch fail!");
        ui_sprite_chipmunk_env_free(world);
        return NULL;
    }
    
    return ui_sprite_world_res_from_data(env);
}

static int ui_sprite_chipmunk_env_create_debug_render(ui_sprite_chipmunk_env_t env, ui_sprite_world_t world, const char * layer_name) {
    ui_sprite_render_env_t render_env;
    ui_sprite_render_layer_t render_layer;
    ui_sprite_render_anim_t render_anim;

    render_env = ui_sprite_render_env_find(world);
    if (render_env == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_chipmunk_env: debug-layer: world no render env!");
        return -1;
    }

    render_layer = ui_sprite_render_layer_find(render_env, layer_name);
    if (render_layer == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_chipmunk_env: debug-layer: layer %s not exist!", layer_name);
        return -1;
    }

    render_anim = ui_sprite_render_anim_create_by_res(render_layer, "chipmunk:all", NULL, "");
    if (render_anim == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_chipmunk_env: debug-layer: create render obj fail!");
        return -1;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
