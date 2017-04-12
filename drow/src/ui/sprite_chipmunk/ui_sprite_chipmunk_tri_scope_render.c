#include <assert.h>
#include <stdio.h>
#include "chipmunk/chipmunk_private.h"
#include "cpe/utils/math_ex.h"
#include "cpe/pal/pal_strings.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "plugin/chipmunk/plugin_chipmunk_render.h"
#include "ui_sprite_chipmunk_tri_scope_render_i.h"
#include "ui_sprite_chipmunk_env_i.h"
#include "ui_sprite_chipmunk_module_i.h"

int ui_sprite_chipmunk_tri_scope_render_do_init(void * ctx, ui_runtime_render_obj_t tri_scope_render_obj) {
    ui_sprite_chipmunk_tri_scope_render_t obj = (ui_sprite_chipmunk_tri_scope_render_t)ui_runtime_render_obj_data(tri_scope_render_obj);
    obj->m_env = NULL;
    return 0;
}

int ui_sprite_chipmunk_tri_scope_render_do_tri_scope_render(
    void * i_ctx, ui_runtime_render_obj_t tri_scope_render_obj,
    ui_runtime_render_t context, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    ui_sprite_chipmunk_tri_scope_render_t obj = (ui_sprite_chipmunk_tri_scope_render_t)ui_runtime_render_obj_data(tri_scope_render_obj);
    ui_sprite_chipmunk_tri_scope_t scope;
    
	if (obj->m_env == NULL) return -1;

    TAILQ_FOREACH(scope, &obj->m_env->m_scopes, m_next_for_env) {
        cpBody body;
        union {
            struct cpCircleShape m_circle;
            struct cpSegmentShape m_segment;
            struct cpPolyShape m_poly;
        } shape_data;

        bzero(&body, sizeof(body));
        cpBodyInit(&body, 0.0f, 0.0f);
        
        switch(scope->m_type) {
        case ui_sprite_chipmunk_tri_scope_dynamic:
            assert(scope->m_dynamic.m_fun);
            if (scope->m_dynamic.m_fun(scope->m_dynamic.m_ctx, &shape_data, &body) != ui_sprite_chipmunk_tri_scope_update_success) {
                cpBodyDestroy(&body);
                continue;
            }
            break;
        case ui_sprite_chipmunk_tri_scope_fix:
            CPE_ERROR(scope->m_env->m_module->m_em, "ui_sprite_chipmunk_tri_scope_query: not support fix yet");
            cpBodyDestroy(&body);
            continue;
        case ui_sprite_chipmunk_tri_scope_unknown:
        default:
            cpBodyDestroy(&body);
            continue;
        }

        cpShapeCacheBB((cpShape *)&shape_data);
        
        plugin_chipmunk_render_shape(context, clip_rect, transform, &shape_data);

        cpShapeDestroy((cpShape*)&shape_data);
        cpBodyDestroy(&body);
    }
    
    return 0;
}

void ui_sprite_chipmunk_tri_scope_render_set_env(ui_sprite_chipmunk_tri_scope_render_t tri_scope_render, ui_sprite_chipmunk_env_t env) {
    tri_scope_render->m_env = env;
}

int ui_sprite_chipmunk_tri_scope_render_regist(ui_sprite_chipmunk_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                module->m_runtime, "chipmunk-tri-scope", 0, sizeof(struct ui_sprite_chipmunk_tri_scope_render), module,
                ui_sprite_chipmunk_tri_scope_render_do_init,
                NULL,
                NULL,
                NULL,
                NULL,
                ui_sprite_chipmunk_tri_scope_render_do_tri_scope_render,
                NULL,
                NULL,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_tri_scope_render_regist: register tri_scope_render obj fail");
            return -1;
        }
    }

    return 0;
}

void ui_sprite_chipmunk_tri_scope_render_unregist(ui_sprite_chipmunk_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        if ((obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "chipmunk-tri-scope"))) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}
