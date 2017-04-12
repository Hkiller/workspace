#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/tiledmap/plugin_tiledmap_render_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_layer.h"
#include "ui/sprite_render/ui_sprite_render_module.h"
#include "ui_sprite_tiledmap_render_creator_i.h"
#include "ui_sprite_tiledmap_env_i.h"

static int ui_sprite_tiledmap_render_create(
    ui_runtime_render_obj_ref_t * r, void * ctx, ui_sprite_world_t world, uint32_t entity_id, const char * res)
{
    ui_sprite_tiledmap_module_t module = ctx;
    ui_sprite_tiledmap_env_t tiledmap_env;
    ui_runtime_render_obj_ref_t render_obj_ref;
    ui_runtime_render_obj_t render_obj;
    plugin_tiledmap_render_layer_t tiledmap_render;
    plugin_tiledmap_layer_t tiledmap_layer;
    
    tiledmap_env = ui_sprite_tiledmap_env_find(world);
    if (tiledmap_env == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tiledmap_render_create: no tiledmap env in world!");
        return -1;
    }

    tiledmap_layer = plugin_tiledmap_layer_find(tiledmap_env->m_env, res);
    if (tiledmap_layer == NULL) {
        tiledmap_layer = plugin_tiledmap_layer_create(tiledmap_env->m_env, res);
        if (tiledmap_layer == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_tiledmap_render_create: tiledmap layer %s create fail!", res);
            return -1;
        }
    }
        
    render_obj = ui_runtime_render_obj_create_by_type(module->m_runtime, NULL, "tiledmap-layer");
    if (render_obj == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tiledmap_render_create: create tiledmap render fail!");
        return -1;
    }

    tiledmap_render = ui_runtime_render_obj_data(render_obj);
    plugin_tiledmap_render_layer_set_layer(tiledmap_render, tiledmap_layer);

    render_obj_ref = ui_runtime_render_obj_ref_create_by_obj(render_obj);
    if (render_obj_ref == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tiledmap_render_create: create obj ref fail!");
        ui_runtime_render_obj_free(render_obj);
        return -1;
    }
    
    *r = render_obj_ref;
    
    return 0;
}

int ui_sprite_tiledmap_render_creator_regist(ui_sprite_tiledmap_module_t module) {
    if (module->m_sprite_render) {
        if (ui_sprite_render_module_register_obj_creator(
                module->m_sprite_render, "tiledmap-layer", ui_sprite_tiledmap_render_create, module)
            != 0)
        {
            CPE_ERROR(module->m_em, "ui_sprite_tiledmap_render_create_regist: register fail!");
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_tiledmap_render_creator_unregist(ui_sprite_tiledmap_module_t module) {
    if (module->m_sprite_render) {
        ui_sprite_render_module_unregister_obj_creator(module->m_sprite_render, "tiledmap-layer");
    }
}
