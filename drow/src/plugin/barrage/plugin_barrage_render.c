#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_color.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "plugin_barrage_render_i.h"
#include "plugin_barrage_bullet_i.h"
#include "plugin_barrage_env_i.h"
#include "plugin_barrage_group_i.h"

int plugin_barrage_render_do_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_barrage_render_t obj = (plugin_barrage_render_t)ui_runtime_render_obj_data(render_obj);
    obj->m_env = NULL;
    obj->m_barrage_group[0] = 0;
    return 0;
}

int plugin_barrage_render_do_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t context, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_barrage_module_t module = (plugin_barrage_module_t)ctx;
    plugin_barrage_render_t obj = (plugin_barrage_render_t)ui_runtime_render_obj_data(render_obj);
    int rv = 0;

    if (obj->m_env == NULL) return -1;

    if (obj->m_barrage_group[0]) {
        plugin_barrage_group_t barrage_group = plugin_barrage_group_find(obj->m_env, obj->m_barrage_group);
        if (barrage_group == NULL) {
            CPE_ERROR(module->m_em, "plugin_barrage_env_render: group %s not exist!", obj->m_barrage_group);
            return -1;
        }

        ui_runtime_render_obj_render(ui_runtime_render_obj_from_data(barrage_group->m_particle_obj), context, clip_rect, transform);
    }
    else {
        plugin_barrage_group_t barrage_group;

        TAILQ_FOREACH(barrage_group, &obj->m_env->m_groups, m_next_for_env) {
            ui_runtime_render_obj_render(ui_runtime_render_obj_from_data(barrage_group->m_particle_obj), context, clip_rect, transform);
        }
    }
    
    return rv;
}

void plugin_barrage_render_set_env(plugin_barrage_render_t render, plugin_barrage_env_t env) {
    render->m_env = env;
}

void plugin_barrage_render_set_group(plugin_barrage_render_t render, const char * group_name) {
    cpe_str_dup(render->m_barrage_group, sizeof(render->m_barrage_group), group_name);
}

int plugin_barrage_render_regist(plugin_barrage_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                module->m_runtime, "barrage", 0, sizeof(struct plugin_barrage_render), module,
                plugin_barrage_render_do_init,
                NULL,
                NULL,
                NULL,
                NULL,
                plugin_barrage_render_do_render,
                NULL,
                NULL,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(module->m_em, "plugin_barrage_render_regist: register render obj fail");
            return -1;
        }
    }

    return 0;
}

void plugin_barrage_render_unregist(plugin_barrage_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        if ((obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "barrage"))) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}
