#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_transform.h"
#include "render/cache/ui_cache_color.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "plugin_basicanim_color_i.h"
#include "plugin_basicanim_utils_i.h"

int plugin_basicanim_color_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    struct plugin_basicanim_color * obj = ui_runtime_render_obj_data(render_obj);
    obj->m_rect = UI_RECT_ZERO;
    obj->m_color = UI_COLOR_WHITE;
    return 0;
}

ui_color_t plugin_basicanim_color_color(plugin_basicanim_color_t anim_color) {
    return &anim_color->m_color;
}

void plugin_basicanim_color_set_color(plugin_basicanim_color_t anim_color,  ui_color_t color) {
    anim_color->m_color = *color;
}

ui_rect_t plugin_basicanim_color_rect(plugin_basicanim_color_t anim_color) {
    return &anim_color->m_rect;
}

void plugin_basicanim_color_set_rect(plugin_basicanim_color_t anim_color,  ui_rect_t rect) {
    anim_color->m_rect = *rect;
}

int plugin_basicanim_color_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t t)
{
    plugin_basicanim_color_t obj = ui_runtime_render_obj_data(render_obj);
    struct ui_color c = obj->m_color;

    if (second_color) ui_runtime_render_second_color_mix(second_color, &c);
    
    /* printf("xxxxx: color=(%f,%f,%f,%f), mix=%d, rect=(%f,%f)-(%f,%f), clip-rect=(%f,%f)-(%f,%f)\n", */
    /*        c.r, c.g, c.b, c.a, second_color->m_mix, */
    /*        obj->m_rect.lt.x, obj->m_rect.lt.y, obj->m_rect.rb.x, obj->m_rect.rb.y, */
    /*        clip_rect->lt.x, clip_rect->lt.y, clip_rect->rb.x, clip_rect->rb.y); */
    
    plugin_basicanim_render_draw_color(context, 0.0f, clip_rect, t, &obj->m_rect, &c);
    return 0;
}

static int plugin_basicanim_color_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * args) {
    plugin_basicanim_color_t obj = (plugin_basicanim_color_t)ui_runtime_render_obj_data(render_obj);
    plugin_basicanim_module_t module = ctx;
    char * str_value;
    int rv = 0;

    if ((str_value = cpe_str_read_and_remove_arg(args, "color.color", ',', '='))) {
        if (ui_cache_find_color(ui_runtime_module_cache_mgr(module->m_runtime), str_value, &obj->m_color) != 0) {
            CPE_ERROR(
                module->m_em, "color obj(%s): color %s error!",
                ui_runtime_render_obj_name(render_obj), str_value);
            rv = -1;
        }
    }
    
    return rv;
}

static void plugin_basicanim_color_bounding(void * ctx, ui_runtime_render_obj_t render_obj, ui_rect_t bounding) {
    struct plugin_basicanim_color * obj = ui_runtime_render_obj_data(render_obj);
    *bounding = obj->m_rect;
}

static int plugin_basicanim_color_resize(void * ctx, ui_runtime_render_obj_t render_obj, ui_vector_2_t size) {
    struct plugin_basicanim_color * obj = ui_runtime_render_obj_data(render_obj);
    obj->m_rect.rb.x = obj->m_rect.lt.x + size->x;
    obj->m_rect.rb.y = obj->m_rect.lt.y + size->y;
    return 0;
}

int plugin_basicanim_color_register(plugin_basicanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta;

    obj_meta =
        ui_runtime_render_obj_meta_create(
            module->m_runtime, "color", 0, sizeof(struct plugin_basicanim_color), module,
            plugin_basicanim_color_init,
            NULL,
            plugin_basicanim_color_setup,
            NULL,
            NULL,
            plugin_basicanim_color_render,
            NULL,
            plugin_basicanim_color_bounding,
            plugin_basicanim_color_resize);
    if (obj_meta == NULL) {
        CPE_ERROR(module->m_em, "%s: create: register render obj fail", plugin_basicanim_module_name(module));
        return -1;
    }

    return 0;
}

void plugin_basicanim_color_unregister(plugin_basicanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_id(module->m_runtime, UI_OBJECT_TYPE_IMG_BLOCK);
    if (obj_meta) {
        ui_runtime_render_obj_meta_free(obj_meta);
    }
}
