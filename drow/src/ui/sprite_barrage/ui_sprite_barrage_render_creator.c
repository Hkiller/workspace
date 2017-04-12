#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/barrage/plugin_barrage_render.h"
#include "ui/sprite_render/ui_sprite_render_module.h"
#include "ui_sprite_barrage_render_creator_i.h"
#include "ui_sprite_barrage_env_i.h"

static int ui_sprite_barrage_render_create(
    ui_runtime_render_obj_ref_t * r, void * ctx, ui_sprite_world_t world, uint32_t entity_id, const char * res)
{
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_env_t barrage_env;
    ui_runtime_render_obj_ref_t render_obj_ref;
    ui_runtime_render_obj_t render_obj;
    plugin_barrage_render_t barrage_render;

    barrage_env = ui_sprite_barrage_env_find(world);
    if (barrage_env == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_barrage_render_create: no barrage env in world!");
        return -1;
    }

    render_obj = ui_runtime_render_obj_create_by_type(module->m_runtime, NULL, "barrage");
    if (render_obj == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_barrage_render_create: create barrage render fail!");
        return -1;
    }

    barrage_render = ui_runtime_render_obj_data(render_obj);
    plugin_barrage_render_set_env(barrage_render, barrage_env->m_env);
    if (res) plugin_barrage_render_set_group(barrage_render, res);

    render_obj_ref = ui_runtime_render_obj_ref_create_by_obj(render_obj);
    if (render_obj_ref == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_barrage_render_create: create obj ref fail!");
        ui_runtime_render_obj_free(render_obj);
        return -1;
    }
    
    *r = render_obj_ref;
    
    return 0;
}

int ui_sprite_barrage_render_creator_regist(ui_sprite_barrage_module_t module) {
    if (module->m_sprite_render) {
        if (ui_sprite_render_module_register_obj_creator(
                module->m_sprite_render, "barrage", ui_sprite_barrage_render_create, module)
            != 0)
        {
            CPE_ERROR(module->m_em, "ui_sprite_barrage_render_create_regist: register fail!");
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_barrage_render_creator_unregist(ui_sprite_barrage_module_t module) {
    if (module->m_sprite_render) {
        ui_sprite_render_module_unregister_obj_creator(module->m_sprite_render, "barrage");
    }
}
