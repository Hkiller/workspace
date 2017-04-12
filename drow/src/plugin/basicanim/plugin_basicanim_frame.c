#include <assert.h>
#include "gd/app/app_log.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_transform.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_sprite.h"
#include "render/model/ui_data_module.h"
#include "render/cache/ui_cache_res.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_cmd_utils_2d.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "plugin_basicanim_frame_i.h"
#include "plugin_basicanim_utils_i.h"

int plugin_basicanim_frame_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    struct plugin_basicanim_frame * obj = ui_runtime_render_obj_data(render_obj);

    obj->m_frame = NULL;

    return 0;
}

ui_data_frame_t plugin_basicanim_frame_data_frame(plugin_basicanim_frame_t frame) {
    return frame->m_frame;
}

int plugin_basicanim_frame_set(void * ctx, ui_runtime_render_obj_t render_obj, UI_OBJECT_URL const * obj_url) {
    plugin_basicanim_module_t module = ctx;
    struct plugin_basicanim_frame * obj = ui_runtime_render_obj_data(render_obj);
    UI_OBJECT_URL_DATA_SRC_ID const * frame_data = &obj_url->data.frame;
    ui_data_src_t src;
    ui_data_sprite_t data_sprite;

    src = ui_runtime_module_find_src(module->m_runtime, &frame_data->src, ui_data_src_type_sprite);
    if (src == NULL) return -1;

    data_sprite = ui_data_src_product(src);
    if (data_sprite == NULL) {
        CPE_ERROR(module->m_em, "src %s not loaded", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return -1;
    }

    if (frame_data->id != (uint32_t)-1) {
        obj->m_frame = ui_data_frame_find_by_id(data_sprite, frame_data->id);
        if (obj->m_frame == NULL) {
            CPE_ERROR(module->m_em, "src %s no frame %d", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), frame_data->id);
            return -1;
        }
    }
    else if (frame_data->name[0]) {
        obj->m_frame = ui_data_frame_find_by_name(data_sprite, frame_data->name);
        if (obj->m_frame == NULL) {
            CPE_ERROR(module->m_em, "src %s no frame %s", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), frame_data->name);
            return -1;
        }
    }
    else {
        CPE_ERROR(module->m_em, "obj url only set src %s, no id or name", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return -1;
    }

    ui_runtime_render_obj_set_src(render_obj, src);

    return 0;
}

static int plugin_basicanim_frame_render_i(
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd, ui_rect_t clip_rect,
    ui_transform_t transform, ui_runtime_render_second_color_t second_color, ui_rect_t render_rect, 
    ui_data_img_block_t img_block, ui_runtime_render_texture_filter_t filter, error_monitor_t em)
{
    UI_IMG_BLOCK const * img_block_data = ui_data_img_block_data(img_block);
    ui_rect src_rect;
    ui_transform local_trans = UI_TRANSFORM_IDENTITY;
    ui_vector_3 local_s;
    ui_cache_res_t texture;
    
    if (!ui_rect_is_valid(render_rect)) return 0;

    texture = ui_data_img_block_using_texture(img_block);
    if (texture == NULL) {
        CPE_ERROR(
            em, "src %s use texture %s not exsit",
            ui_data_src_path_dump(gd_app_tmp_buffer(ui_runtime_render_app(render)), ui_data_img_block_src(img_block)),
            ui_data_img_block_using_texture_path(img_block));
        return -1;
    }

    ui_transform_set_pos_2(&local_trans, &render_rect->lt);
    
    local_s.x = ui_rect_width(render_rect) / img_block_data->src_w;
    local_s.y = ui_rect_height(render_rect) / img_block_data->src_h;
    local_s.z = 1.0f;
    ui_transform_set_scale(&local_trans, &local_s);
    
    src_rect.lt.x = img_block_data->src_x;
    src_rect.lt.y = img_block_data->src_y;
    src_rect.rb.x = img_block_data->src_x + img_block_data->src_w;
    src_rect.rb.y = img_block_data->src_y + img_block_data->src_h;

    ui_transform_adj_by_parent(&local_trans, transform);
    
    plugin_basicanim_render_draw_rect(
        render, batch_cmd, clip_rect, &local_trans, second_color,
        texture, &src_rect, filter);
    
    return 0;
}

int plugin_basicanim_frame_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t render, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_basicanim_module_t module = ctx;
    struct plugin_basicanim_frame * obj = ui_runtime_render_obj_data(render_obj);
    ui_runtime_render_cmd_t batch_cmd = NULL;
    int rv = 0;
    
    if (obj->m_frame == NULL) return -1;

    if (plugin_basicanim_render_draw_frame(render, &batch_cmd, clip_rect, transform, obj->m_frame, second_color, module->m_em) != 0) {
        rv = -1;
    }

    if (ui_runtime_render_cmd_quad_batch_commit(&batch_cmd, render) != 0) {
        rv = -1;
    }

    return rv;
}

int plugin_basicanim_render_draw_frame(
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd, ui_rect_t clip_rect,
    ui_transform_t transform, ui_data_frame_t frame, ui_runtime_render_second_color_t second_color,
    error_monitor_t em)
{
    UI_FRAME const * frame_data;
    int rv = 0;
    
    frame_data = ui_data_frame_data(frame);

    if (frame_data->accept_scale) {
        /*九宫格图片 */
        struct ui_data_frame_img_it frame_img_it;
        struct {
            ui_data_img_block_t block;
            ui_runtime_render_texture_filter_t filter;
            ui_vector_2 sz;
        } imgs[9];
        uint8_t i;
        ui_rect center;
        ui_vector_2 full_sz;
        
        ui_data_frame_imgs(&frame_img_it, frame);
        for(i = 0; i < CPE_ARRAY_SIZE(imgs); ++i) {
            ui_data_frame_img_t frame_img;
            UI_IMG_REF const * frame_img_data;

            frame_img = ui_data_frame_img_it_next(&frame_img_it);
            if (frame_img == NULL) break;

            frame_img_data = ui_data_frame_img_data(frame_img);

            imgs[i].block = ui_data_frame_img_using_img_block(frame_img);
            if (imgs[i].block) {
                UI_IMG_BLOCK const * block_data = ui_data_img_block_data(imgs[i].block);
                imgs[i].sz.x = block_data->src_w;
                imgs[i].sz.y = block_data->src_h;
            }
            else {
                imgs[i].sz = UI_VECTOR_2_ZERO;
            }

            imgs[i].filter = frame_img_data->trans.filter ==0 ? ui_runtime_render_filter_nearest : ui_runtime_render_filter_linear;
        }

        /*计算整体大小 */
        if (!frame_data->bound_custom) {
            full_sz.x = imgs[ui_frame_img_grid_pos_lc].sz.x + imgs[ui_frame_img_grid_pos_cc].sz.x + imgs[ui_frame_img_grid_pos_rc].sz.x;
            full_sz.y = imgs[ui_frame_img_grid_pos_ct].sz.y + imgs[ui_frame_img_grid_pos_cc].sz.y + imgs[ui_frame_img_grid_pos_cb].sz.y;
        }
        else {
            full_sz.x = frame_data->bounding.rt - frame_data->bounding.lt;
            full_sz.y = frame_data->bounding.bm - frame_data->bounding.tp;
        }

        /*计算中间部分的大小 */
        center.lt.x = imgs[ui_frame_img_grid_pos_lc].sz.x / transform->m_s.x;
        center.lt.y = imgs[ui_frame_img_grid_pos_ct].sz.y / transform->m_s.y;
        center.rb.x = full_sz.x - imgs[ui_frame_img_grid_pos_rc].sz.x / transform->m_s.x;
        center.rb.y = full_sz.y - imgs[ui_frame_img_grid_pos_cb].sz.y / transform->m_s.y;

        if (center.lt.x > center.rb.x) {
            center.lt.x = imgs[ui_frame_img_grid_pos_lc].sz.x;
            center.rb.x = full_sz.x - imgs[ui_frame_img_grid_pos_rc].sz.x;
        }

        if (center.lt.y > center.rb.y) {
            center.lt.y = imgs[ui_frame_img_grid_pos_ct].sz.y;
            center.rb.y = full_sz.y - imgs[ui_frame_img_grid_pos_cb].sz.y;
        }

        /*左上角 */
        if (imgs[ui_frame_img_grid_pos_lt].block) {
            ui_rect rect = UI_RECT_INITLIZER(0.0f, 0.0f, center.lt.x, center.lt.y);
            
            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    imgs[ui_frame_img_grid_pos_lt].block, imgs[ui_frame_img_grid_pos_lt].filter, em) != 0)
            {
                rv = -1;
            }
        }

        /*右上角 */
        if (imgs[ui_frame_img_grid_pos_rt].block) {
            ui_rect rect = UI_RECT_INITLIZER(center.rb.x, 0.0f, full_sz.x, center.lt.y);
            
            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    imgs[ui_frame_img_grid_pos_rt].block, imgs[ui_frame_img_grid_pos_rt].filter, em) != 0)
            {
                rv = -1;
            }
        }

        /*左下角 */
        if (imgs[ui_frame_img_grid_pos_lb].block) {
            ui_rect rect = UI_RECT_INITLIZER(0.0f, center.rb.y, center.lt.x, full_sz.y);
            
            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    imgs[ui_frame_img_grid_pos_lb].block, imgs[ui_frame_img_grid_pos_lb].filter, em) != 0)
            {
                rv = -1;
            }
        }

        /*右下角 */
        if (imgs[ui_frame_img_grid_pos_rb].block) {
            ui_rect rect = UI_RECT_INITLIZER(center.rb.x, center.rb.y, full_sz.x, full_sz.y);
            
            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    imgs[ui_frame_img_grid_pos_rb].block, imgs[ui_frame_img_grid_pos_rb].filter, em) != 0)
            {
                rv = -1;
            }
        }

        /*左中 */
        if (imgs[ui_frame_img_grid_pos_lc].block) {
            ui_rect rect = UI_RECT_INITLIZER(0.0f, center.lt.y, center.lt.x, center.rb.y);
            
            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    imgs[ui_frame_img_grid_pos_lc].block, imgs[ui_frame_img_grid_pos_lc].filter, em) != 0)
            {
                rv = -1;
            }
        }

        /*右中 */
        if (imgs[ui_frame_img_grid_pos_rc].block) {
            ui_rect rect = UI_RECT_INITLIZER(center.rb.x, center.lt.y, full_sz.x, center.rb.y);
            
            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    imgs[ui_frame_img_grid_pos_rc].block, imgs[ui_frame_img_grid_pos_rc].filter, em) != 0)
            {
                rv = -1;
            }
        }

        /*中上 */
        if (imgs[ui_frame_img_grid_pos_ct].block) {
            ui_rect rect = UI_RECT_INITLIZER(center.lt.x, 0.0f, center.rb.x, center.lt.y);
            
            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    imgs[ui_frame_img_grid_pos_ct].block, imgs[ui_frame_img_grid_pos_rc].filter, em) != 0)
            {
                rv = -1;
            }
        }

        /*中下 */
        if (imgs[ui_frame_img_grid_pos_cb].block) {
            ui_rect rect = UI_RECT_INITLIZER(center.lt.x, center.rb.y, center.rb.x, full_sz.y);
            
            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    imgs[ui_frame_img_grid_pos_cb].block, imgs[ui_frame_img_grid_pos_cb].filter, em) != 0)
            {
                rv = -1;
            }
        }

        /*中间块 */
        if (imgs[ui_frame_img_grid_pos_cc].block) {
            ui_rect rect = UI_RECT_INITLIZER(center.lt.x, center.lt.y, center.rb.x, center.rb.y);
            
            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    imgs[ui_frame_img_grid_pos_cc].block, imgs[ui_frame_img_grid_pos_cc].filter,em) != 0)
            {
                rv = -1;
            }
        }
    }
    else {
        struct ui_data_frame_img_it frame_img_it;
        ui_data_frame_img_t frame_img;
        ui_runtime_render_texture_filter_t filter;

        ui_data_frame_imgs(&frame_img_it, frame);
        while((frame_img = ui_data_frame_img_it_next(&frame_img_it))) {
            ui_data_img_block_t img;
            ui_rect rect;
            UI_IMG_REF const * img_ref_data;
            UI_IMG_BLOCK const * img_data;
            
            img = ui_data_frame_img_using_img_block(frame_img);
            if (img == NULL) continue;

            img_ref_data = ui_data_frame_img_data(frame_img);
            img_data = ui_data_img_block_data(img);
            filter = img_ref_data->trans.filter ==0 ? ui_runtime_render_filter_nearest : ui_runtime_render_filter_linear;
            
            switch(img_ref_data->trans.world_trans.flips ^ img_ref_data->trans.local_trans.flips) {
            case 0:
                rect.lt.x = img_ref_data->trans.world_trans.trans.value[0];
                rect.lt.y = img_ref_data->trans.world_trans.trans.value[1];
                rect.rb.x = rect.lt.x + img_data->src_w * img_ref_data->trans.world_trans.scale.value[0];
                rect.rb.y = rect.lt.y + img_data->src_h * img_ref_data->trans.world_trans.scale.value[1];
                break;
            case 1:
                //rect.lt.x = img_ref_data->trans.world_trans.trans.value[0] - img_data->src_w * img_ref_data->trans.world_trans.scale.value[0];
                rect.lt.x = img_ref_data->trans.world_trans.trans.value[0] - 2 * img_data->src_w * img_ref_data->trans.world_trans.scale.value[0];
                rect.lt.y = img_ref_data->trans.world_trans.trans.value[1];
                //rect.rb.x = img_ref_data->trans.world_trans.trans.value[0];
                rect.rb.x = rect.lt.x + img_data->src_w * img_ref_data->trans.world_trans.scale.value[0];
                rect.rb.y = rect.lt.y + img_data->src_h * img_ref_data->trans.world_trans.scale.value[1];
                transform->m_s.x *= -1;
                break;
            case 2:
                rect.lt.x = img_ref_data->trans.world_trans.trans.value[0];
                //rect.lt.y = img_ref_data->trans.world_trans.trans.value[1] - img_data->src_h * img_ref_data->trans.world_trans.scale.value[1];
                rect.lt.y = img_ref_data->trans.world_trans.trans.value[1] - 2 * img_data->src_h * img_ref_data->trans.world_trans.scale.value[1];
                rect.rb.x = rect.lt.x + img_data->src_w * img_ref_data->trans.world_trans.scale.value[0];
                //rect.rb.y = img_ref_data->trans.world_trans.trans.value[1];
                rect.rb.y = rect.lt.y + img_data->src_h * img_ref_data->trans.world_trans.scale.value[1];
                transform->m_s.y *= -1;
                break;
            case 3:
                //rect.lt.x = img_ref_data->trans.world_trans.trans.value[0] - img_data->src_w * img_ref_data->trans.world_trans.scale.value[0];
                //rect.lt.y = img_ref_data->trans.world_trans.trans.value[1] - img_data->src_h * img_ref_data->trans.world_trans.scale.value[1];
                //rect.rb.x = img_ref_data->trans.world_trans.trans.value[0];
                //rect.rb.y = img_ref_data->trans.world_trans.trans.value[1];
                rect.lt.x = img_ref_data->trans.world_trans.trans.value[0] - 2 * img_data->src_w * img_ref_data->trans.world_trans.scale.value[0];
                rect.lt.y = img_ref_data->trans.world_trans.trans.value[1] - 2 * img_data->src_h * img_ref_data->trans.world_trans.scale.value[1];
                rect.rb.x = rect.lt.x + img_data->src_w * img_ref_data->trans.world_trans.scale.value[0];
                rect.rb.y = rect.lt.y + img_data->src_h * img_ref_data->trans.world_trans.scale.value[1];
                transform->m_s.x *= -1;
                transform->m_s.y *= -1;
                break;
            }

            if (plugin_basicanim_frame_render_i(
                    render, batch_cmd, clip_rect, transform, second_color, &rect,
                    img, filter, em) != 0)
            {
                rv = -1;
            }
        }
    }
    
    return rv;
}

static void plugin_basicanim_frame_bounding(void * ctx, ui_runtime_render_obj_t render_obj, ui_rect_t bounding) {
    struct plugin_basicanim_frame * obj = ui_runtime_render_obj_data(render_obj);
    if (obj->m_frame) {
        ui_data_frame_bounding_rect(obj->m_frame, bounding);
    }
    else {
        *bounding = UI_RECT_ZERO;
    }
}

int plugin_basicanim_frame_register(plugin_basicanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta;

    obj_meta =
        ui_runtime_render_obj_meta_create(
            module->m_runtime, "frame", UI_OBJECT_TYPE_FRAME, sizeof(struct plugin_basicanim_frame), module,
            plugin_basicanim_frame_init,
            plugin_basicanim_frame_set,
            NULL,
            NULL,
            NULL,
            plugin_basicanim_frame_render,
            NULL,
            plugin_basicanim_frame_bounding,
            NULL);
    if (obj_meta == NULL) {
        CPE_ERROR(module->m_em, "%s: create: register render obj fail", plugin_basicanim_module_name(module));
        return -1;
    }

    return 0;
}

void plugin_basicanim_frame_unregister(plugin_basicanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_id(module->m_runtime, UI_OBJECT_TYPE_FRAME);
    if (obj_meta) {
        ui_runtime_render_obj_meta_free(obj_meta);
    }
}
