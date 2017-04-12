#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_transform.h"
#include "render/cache/ui_cache_res.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "plugin/basicanim/plugin_basicanim_utils.h"
#include "plugin_remoteanim_obj_i.h"
#include "plugin_remoteanim_block_i.h"

static int plugin_remoteanim_obj_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_remoteanim_module_t module = ctx;
    plugin_remoteanim_obj_t obj = ui_runtime_render_obj_data(render_obj);

    obj->m_module = module;
    obj->m_block = NULL;
    obj->m_resize_to_default = 1;
    obj->m_default = NULL;
    return 0;
}

static void plugin_remoteanim_obj_fini(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_remoteanim_obj_t obj = ui_runtime_render_obj_data(render_obj);

    if (obj->m_block) plugin_remoteanim_obj_set_block(obj, NULL);
    if (obj->m_default) ui_runtime_render_obj_ref_free(obj->m_default);
}

ui_runtime_render_obj_ref_t plugin_remoteanim_obj_default(plugin_remoteanim_obj_t obj) {
    return obj->m_default;
}

void plugin_remoteanim_obj_set_default(plugin_remoteanim_obj_t obj, ui_runtime_render_obj_ref_t obj_ref) {
    if (obj->m_default) {
        ui_runtime_render_obj_ref_free(obj->m_default);
    }

    obj->m_default = obj_ref;
}

int plugin_remoteanim_obj_set_default_by_res(plugin_remoteanim_obj_t obj, const char * res, char * addition_args) {
    plugin_remoteanim_module_t module = obj->m_module;

    obj->m_default = ui_runtime_render_obj_ref_create_by_res(module->m_runtime, res, NULL);
    if (obj->m_default == NULL) {
        CPE_ERROR(
            module->m_em, "remote-pic obj %s: create dft obj ref fail!",
            ui_runtime_render_obj_name(ui_runtime_render_obj_from_data(obj)));
        return -1;
    }

    if (addition_args && ui_runtime_render_obj_ref_setup(obj->m_default, addition_args) != 0) {
        ui_runtime_render_obj_ref_free(obj->m_default);
        obj->m_default = NULL;
        CPE_ERROR(
            module->m_em, "remote-pic obj %s: create dft obj setup fail from %s!",
            ui_runtime_render_obj_name(ui_runtime_render_obj_from_data(obj)), addition_args);
        return -1;
    }

    return 0;
}

plugin_remoteanim_block_t plugin_remoteanim_obj_block(plugin_remoteanim_obj_t obj) {
    return obj->m_block;
}

void plugin_remoteanim_obj_set_block(plugin_remoteanim_obj_t obj, plugin_remoteanim_block_t block) {
    if (obj->m_block) {
        TAILQ_REMOVE(&obj->m_block->m_objs, obj, m_next_for_block);
        if (obj->m_block->m_group->m_auto_free && TAILQ_EMPTY(&obj->m_block->m_objs)) {
            plugin_remoteanim_block_free(obj->m_block);
        }
    }

    obj->m_block = block;

    if (obj->m_block) {
        TAILQ_INSERT_TAIL(&obj->m_block->m_objs, obj, m_next_for_block);
    }
}

int plugin_remoteanim_obj_set_block_by_def(plugin_remoteanim_obj_t obj, const char * block_name, const char * url) {
    const char * sep;
    char group_name_buf[32];
    plugin_remoteanim_group_t group;
    plugin_remoteanim_block_t block;
    uint8_t block_is_create = 0;
    
    sep = strchr(block_name, '.');
    if (sep == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_remoteanim_obj_set_block_by_def: block name %s format error");
        return -1;
    }

    cpe_str_dup_range(group_name_buf, sizeof(group_name_buf), block_name, sep);
    sep++;

    group = plugin_remoteanim_group_find(obj->m_module, group_name_buf);
    if (group == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_remoteanim_obj_set_block_by_def: group %s not exist", group_name_buf);
        return -1;
    }

    block = plugin_remoteanim_block_find(group, sep);
    if (block == NULL) {
        block_is_create = 1;
        block = plugin_remoteanim_block_create(group, sep);
        if (block == NULL) {
            CPE_ERROR(obj->m_module->m_em, "plugin_remoteanim_obj_set_block_by_def: group %s block %s create fail", group_name_buf, sep);
            return -1;
        }
    }

    if (plugin_remoteanim_block_start(block, url) != 0) {
        CPE_ERROR(obj->m_module->m_em, "plugin_remoteanim_obj_set_block_by_def: %s start url %s fail", group_name_buf, block_name, url);
        if (block_is_create) plugin_remoteanim_block_free(block);
        return -1;
    }

    plugin_remoteanim_obj_set_block(obj, block);
    
    return 0;
}

static void plugin_remoteanim_obj_bounding(void * ctx, ui_runtime_render_obj_t render_obj, ui_rect_t bounding) {
    plugin_remoteanim_obj_t obj = ui_runtime_render_obj_data(render_obj);

    if (obj->m_default) {
        ui_runtime_render_obj_get_bounding(ui_runtime_render_obj_ref_obj(obj->m_default), bounding);
    }
    else {
        if (obj->m_block && obj->m_block->m_size.x > 0.0f && obj->m_block->m_size.y > 0.0f) {
            bounding->lt = UI_VECTOR_2_ZERO;
            bounding->rb = obj->m_block->m_size;
        }
        else {
            bounding->lt = UI_VECTOR_2_ZERO;
            bounding->rb = UI_VECTOR_2_ZERO;
        }
    }
}

static int plugin_remoteanim_obj_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * args) {
    plugin_remoteanim_obj_t obj = (plugin_remoteanim_obj_t)ui_runtime_render_obj_data(render_obj);
    plugin_remoteanim_module_t module = ctx;
    char * str_value = NULL;
    char * str_block = NULL;
    char * str_url = NULL;
    int rv = 0;

    str_block = cpe_str_read_and_remove_arg(args, "remote.block", ',', '=');
    if (str_block == NULL) {
        CPE_ERROR(
            module->m_em, "remote-pic obj %s: remote.block not exist!",
            ui_runtime_render_obj_name(render_obj));
        rv = -1;
    }

    str_url = cpe_str_read_and_remove_arg(args, "remote.url", ',', '=');
    if (str_url == NULL) {
        CPE_ERROR(
            module->m_em, "remote-pic obj %s: remote.url not exist!",
            ui_runtime_render_obj_name(render_obj));
        rv = -1;
    }

    if (str_block && str_url) {
        if (plugin_remoteanim_obj_set_block_by_def(obj, str_block, str_url) != 0) {
            rv = -1;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(args, "remote.default", ',', '='))) {
        if (plugin_remoteanim_obj_set_default_by_res(obj, str_value, args) != 0) {
            rv = -1;
        }
    }
    
    return rv;
}

int plugin_remoteanim_obj_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t render, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t t)
{
    plugin_remoteanim_obj_t obj = ui_runtime_render_obj_data(render_obj);

    if (obj->m_block && obj->m_block->m_state == plugin_remoteanim_block_state_installed) {
        ui_transform_t using_t = t;
        ui_transform t_buf;
        
        if (obj->m_resize_to_default && obj->m_default) {
            ui_rect default_bounding;
            if (ui_runtime_render_obj_get_bounding(ui_runtime_render_obj_ref_obj(obj->m_default), &default_bounding) == 0) {
                ui_vector_3 s = t->m_s;
                
                s.x *= ui_rect_width(&default_bounding) / obj->m_block->m_size.x;
                s.y *= ui_rect_height(&default_bounding) / obj->m_block->m_size.y;

                t_buf = *t;
                ui_transform_set_scale(&t_buf, &s);
                using_t = &t_buf;
            }
        }

        plugin_basicanim_render_draw_rect(
            render, NULL, clip_rect,
            using_t, second_color,
            obj->m_block->m_group->m_texture,
            &obj->m_block->m_place, ui_runtime_render_filter_linear);
		return 0;
    }
    else if (obj->m_default) {
        ui_runtime_render_obj_ref_set_second_color(obj->m_default, second_color);
        return ui_runtime_render_obj_ref_render(obj->m_default, render, clip_rect, t);
    }
    else {
        return -1;
    }
}

int plugin_remoteanim_obj_register(plugin_remoteanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta;

    obj_meta =
        ui_runtime_render_obj_meta_create(
            module->m_runtime, "remote-pic", 0, sizeof(struct plugin_remoteanim_obj), module,
            plugin_remoteanim_obj_init,
            NULL,
            plugin_remoteanim_obj_setup,
            NULL,
            plugin_remoteanim_obj_fini,
            plugin_remoteanim_obj_render,
            NULL,
            plugin_remoteanim_obj_bounding,
            NULL);
    if (obj_meta == NULL) {
        CPE_ERROR(module->m_em, "%s: create: register render obj fail", plugin_remoteanim_module_name(module));
        return -1;
    }

    return 0;
}

void plugin_remoteanim_obj_unregister(plugin_remoteanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_id(module->m_runtime, UI_OBJECT_TYPE_IMG_BLOCK);
    if (obj_meta) {
        ui_runtime_render_obj_meta_free(obj_meta);
    }
}
