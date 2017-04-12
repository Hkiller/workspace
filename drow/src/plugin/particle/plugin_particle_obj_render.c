#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_color.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_types.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "render/runtime/ui_runtime_render_cmd_utils_2d.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin_particle_obj_i.h"
#include "plugin_particle_data_i.h"
#include "plugin_particle_obj_emitter_i.h"
#include "plugin_particle_obj_particle_i.h"

static void plugin_particle_obj_calc_tiled_texture(
    ui_rect_t block_texture_rect,
    ui_rect_t texture_rect, uint32_t tiling_u, uint32_t tiling_v, uint32_t idx);

static ui_runtime_render_blend_t plugin_particle_obj_calc_blend(
    plugin_particle_module_t module, UI_PARTICLE_EMITTER const * emitter_data, ui_runtime_render_blend_t buf);
static ui_runtime_render_texture_filter_t plugin_particle_obj_calc_filter(
    plugin_particle_module_t module, UI_PARTICLE_EMITTER const * emitter_data);

static void plugin_particle_obj_fill_rect(
    ui_runtime_render_t render,
    ui_transform_t transform,
    ui_runtime_render_second_color_t second_color,  ui_runtime_render_blend_t blend,
    ui_cache_res_t texture, ui_runtime_render_texture_filter_t filter,
    ui_rect_t target, ui_vector_2_t target_block_size,
    ui_rect_t texture_rect, ui_rect_t base_texture_rect);

int plugin_particle_obj_render(
    void * ctx, ui_runtime_render_obj_t obj, ui_runtime_render_t render, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t i_second_color, ui_transform_t t)
{
    plugin_particle_module_t module = ctx;
    plugin_particle_obj_t particle_obj = ui_runtime_render_obj_data(obj);
    plugin_particle_obj_emitter_t emitter;
    plugin_particle_obj_particle_t particle;
    ui_transform world_adj_trans;
    uint8_t world_adj_trans_init = 0;
    
    /*遍历所有的发射器 */
    TAILQ_FOREACH(emitter, &particle_obj->m_emitters, m_next) {
        struct ui_runtime_render_blend blend_buf;
        ui_runtime_render_blend_t blend;
        struct ui_runtime_render_second_color second_color;
        ui_runtime_render_texture_filter_t filter;
        UI_PARTICLE_EMITTER const * emitter_data;
        ui_transform_t emitter_trans;
        ui_vector_2 block_size;
        ui_vector_2 texture_size;
        ui_rect texture_rect;
        ui_rect target_rect;
        
        if (emitter->m_use_state == plugin_particle_obj_emitter_use_state_suspend) continue;

        if (emitter->m_texture == NULL) continue;

        emitter_data = plugin_particle_obj_emitter_data_r(emitter);
        if (emitter_data == NULL) continue;

        if (!emitter_data->is_render) continue;

        blend = plugin_particle_obj_calc_blend(module, emitter_data, &blend_buf);
        filter = plugin_particle_obj_calc_filter(module, emitter_data);
        
        if (emitter_data->xform_mod == UI_PARTICLE_XFORM_LOCAL) {
            emitter_trans = t;
        }
        else {
            if (t) {
                if (!world_adj_trans_init) {
                    ui_transform_t obj_trans = ui_runtime_render_obj_transform(obj);
                    if (cpe_float_cmp(obj_trans->m_s.x, 0.0f, UI_FLOAT_PRECISION) == 0
                        || cpe_float_cmp(obj_trans->m_s.y, 0.0f, UI_FLOAT_PRECISION) == 0)
                    {
                        continue;
                    }
                    
                    ui_transform_reverse(&world_adj_trans, obj_trans);
                    ui_transform_adj_by_parent(&world_adj_trans, t);
                    world_adj_trans_init = 1;
                }

                emitter_trans = &world_adj_trans;
            }
            else {
                emitter_trans = NULL;
            }
        }

        if (emitter_trans) {
            if (cpe_float_cmp(emitter_trans->m_s.x, 0.0f, UI_FLOAT_PRECISION) == 0
                || cpe_float_cmp(emitter_trans->m_s.x, 0.0f, UI_FLOAT_PRECISION) == 0)
            {
                continue;
            }
        }

        texture_size.x = emitter->m_tex_coord_scale.x;
        texture_size.y = emitter->m_tex_coord_scale.y;
        
        texture_rect.lt.x = emitter->m_tex_coord_start.x;
        texture_rect.lt.y = emitter->m_tex_coord_start.y;
        texture_rect.rb.x = texture_rect.lt.x + texture_size.x;
        texture_rect.rb.y = texture_rect.lt.y + texture_size.y;

        target_rect.lt.x = emitter->m_normalized_vtx[1].x * emitter->m_tile_size.x;
        target_rect.lt.y = emitter->m_normalized_vtx[1].y * emitter->m_tile_size.y;
        target_rect.rb.x = emitter->m_normalized_vtx[3].x * emitter->m_tile_size.x;
        target_rect.rb.y = emitter->m_normalized_vtx[3].y * emitter->m_tile_size.y;

        block_size.x = ui_rect_width(&target_rect);
        block_size.y = ui_rect_height(&target_rect);

        if (emitter->m_texture_size.x > emitter->m_texture_origin_size.x) {
            block_size.x *= emitter->m_texture_origin_size.x / emitter->m_texture_size.x;
        }

        if (emitter->m_texture_size.y > emitter->m_texture_origin_size.y) {
            block_size.y *= emitter->m_texture_origin_size.y / emitter->m_texture_size.y;
        }
        
        TAILQ_FOREACH(particle, &emitter->m_particles, m_next) {
            ui_transform o;

            if (particle->m_relative_time == 0.0f) continue;
            
            plugin_particle_obj_particle_calc_transform(particle, &o);
            if (cpe_float_cmp(o.m_s.x, 0.0f, UI_FLOAT_PRECISION) == 0 || cpe_float_cmp(o.m_s.x, 0.0f, UI_FLOAT_PRECISION) == 0) continue;
            
            if (emitter_trans) ui_transform_adj_by_parent(&o, emitter_trans);
            
            /*目标绘制位置 */
            ui_assert_vector_2_sane(&emitter->m_tile_size);

            /* 转换颜色格式 */
            second_color.m_mix = ui_runtime_render_second_color_multiply;
            ui_color_set_from_argb(&second_color.m_color, particle->m_color);

            if(i_second_color != NULL) {
                ui_runtime_render_second_color_mix(i_second_color, &second_color.m_color);
            }

            /*处理UV动画想过参数 */
            if(emitter->m_texture_mode == plugin_particle_obj_emitter_texture_mode_basic) {
                plugin_particle_obj_fill_rect(
                    render, &o, &second_color, blend, emitter->m_texture, filter,
                    &target_rect, &block_size, &texture_rect, &texture_rect);
            }
            else if (emitter->m_texture_mode == plugin_particle_obj_emitter_texture_mode_tiled) {
                ui_rect block_texture_rect;

                plugin_particle_obj_calc_tiled_texture(
                    &block_texture_rect,
                    &texture_rect, emitter_data->tiling_u, emitter_data->tiling_v, particle->m_texture_tile.m_index);

                plugin_particle_obj_fill_rect(
                    render, &o, &second_color, blend, emitter->m_texture, filter,
                    &target_rect, NULL, &block_texture_rect, &block_texture_rect);
            }
            else if (emitter->m_texture_mode == plugin_particle_obj_emitter_texture_mode_scroll) {
                ui_vector_2 render_texture_size = UI_VECTOR_2_INITLIZER(
                    ui_cache_texture_width(emitter->m_texture),
                    ui_cache_texture_height(emitter->m_texture));
                ui_vector_2 full_texture_size = UI_VECTOR_2_INITLIZER(
                    (float)emitter->m_texture_origin_size.x / render_texture_size.x,
                    (float)emitter->m_texture_origin_size.y / render_texture_size.y);
                ui_rect full_texture_rect;
                ui_rect cur_texture_rect;
                ui_vector_2 moved_size;
                ui_vector_2 start_pos_percent;
                    
                full_texture_rect.lt.x = (float)emitter_data->texture_x / render_texture_size.x;
                full_texture_rect.lt.y = (float)emitter_data->texture_y / render_texture_size.y;
                full_texture_rect.rb.x = full_texture_rect.lt.x + full_texture_size.x;
                full_texture_rect.rb.y = full_texture_rect.lt.y + full_texture_size.y;

                //???
                assert(ui_rect_is_contain_pt(&full_texture_rect, &texture_rect.lt));

                assert(ui_rect_width(&texture_rect) <= ui_rect_width(&full_texture_rect) + UI_FLOAT_PRECISION);
                assert(ui_rect_height(&texture_rect) <= ui_rect_height(&full_texture_rect) + UI_FLOAT_PRECISION);

                moved_size.x = (float)(particle->m_texture_scroll.m_u * emitter->m_tile_size.x) / render_texture_size.x;
                moved_size.y = (float)(particle->m_texture_scroll.m_v * emitter->m_tile_size.y) / render_texture_size.y;
                
                start_pos_percent.x = (texture_rect.lt.x + moved_size.x - full_texture_rect.lt.x) / full_texture_size.x;
                start_pos_percent.y = (texture_rect.lt.y + moved_size.y - full_texture_rect.lt.y) / full_texture_size.y;
                start_pos_percent.x -= floor(start_pos_percent.x);
                start_pos_percent.y -= floor(start_pos_percent.y);

                cur_texture_rect.lt.x = full_texture_rect.lt.x + full_texture_size.x * start_pos_percent.x;
                cur_texture_rect.lt.y = full_texture_rect.lt.y + full_texture_size.y * start_pos_percent.y;
                cur_texture_rect.rb.x = cur_texture_rect.lt.x + texture_size.x;
                cur_texture_rect.rb.y = cur_texture_rect.lt.y + texture_size.y;
                
                /* printf("xxxxx: (u,v)=(%f,%f), moved=(%f,%f), start_pos_percent=(%f,%f)\n", */
                /*        particle->m_texture_scroll.m_u, particle->m_texture_scroll.m_v, */
                /*        moved_size.x, moved_size.y, */
                /*        start_pos_percent.x, start_pos_percent.y); */

                plugin_particle_obj_fill_rect(
                    render, &o, &second_color, blend, emitter->m_texture, filter,
                    &target_rect, &block_size, &cur_texture_rect, &full_texture_rect);
            }
            else {
                assert(0);
            }
        }
    }

    return 0;
}

static ui_runtime_render_texture_filter_t
plugin_particle_obj_calc_filter(plugin_particle_module_t module, UI_PARTICLE_EMITTER const * emitter_data) {
    switch(emitter_data->filter_mode) {
    case UI_PARTICLE_FILTER_NEAREST:
        return ui_runtime_render_filter_nearest;
    case UI_PARTICLE_FILTER_LINEAR:
        return ui_runtime_render_filter_linear;
    case UI_PARTICLE_FILTER_NEAREST_MIPMAP_NEAREST:
        return ui_runtime_render_filter_nearest;
    case UI_PARTICLE_FILTER_NEAREST_MIPMAP_LINEAR:
        return ui_runtime_render_filter_linear;
    case UI_PARTICLE_FILTER_LINEAR_MIPMAP_NEAREST:
        return ui_runtime_render_filter_nearest;
    case UI_PARTICLE_FILTER_LINEAR_MIPMAP_LINEAR:
        return ui_runtime_render_filter_linear;
    default:
        CPE_ERROR(module->m_em, "plugin_particle_obj_render: emitter filter mode %d unknown!", emitter_data->blend_mode);
        return ui_runtime_render_filter_linear;
    }
}

static ui_runtime_render_blend_t
plugin_particle_obj_calc_blend(plugin_particle_module_t module, UI_PARTICLE_EMITTER const * emitter_data, ui_runtime_render_blend_t blend_buf) {
    switch(emitter_data->blend_mode) {
    case UI_PARTICLE_BLEND_ADDITIVE:
        blend_buf->m_src_factor = ui_runtime_render_src_alpha;
        blend_buf->m_dst_factor = ui_runtime_render_one;
        break;
    case UI_PARTICLE_BLEND_ALPHABASE:
        blend_buf->m_src_factor = ui_runtime_render_src_alpha;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_alpha;
        break;
    case UI_PARTICLE_BLEND_COLORBASE:
        blend_buf->m_src_factor = ui_runtime_render_one;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_color;
        break;
    case UI_PARTICLE_BLEND_NONE:
        return NULL;
    case UI_PARTICLE_BLEND_BLACK:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_one;
        break;
    case UI_PARTICLE_BLEND_MULTIPLY:
        blend_buf->m_src_factor = ui_runtime_render_zero;
        blend_buf->m_dst_factor = ui_runtime_render_src_color;
        break;
    case UI_PARTICLE_BLEND_MULTIPLY_FILTER:
        blend_buf->m_src_factor = ui_runtime_render_zero;
        blend_buf->m_dst_factor = ui_runtime_render_dst_color;
        break;		
    case UI_PARTICLE_BLEND_DARKROOM:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_alpha;
        break;	
    case UI_PARTICLE_BLEND_DODGE:
        blend_buf->m_src_factor = ui_runtime_render_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_dst_alpha;
        break;
    case UI_PARTICLE_BLEND_DODGE_FILTER:
        blend_buf->m_src_factor = ui_runtime_render_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_dst_color;
        break;
    case UI_PARTICLE_BLEND_FILTER_COLOR:
        blend_buf->m_src_factor = ui_runtime_render_one;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_color;
        break;
    case UI_PARTICLE_BLEND_ADDITIVE_1:
        blend_buf->m_src_factor = ui_runtime_render_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_dst_color;
        break;
    case UI_PARTICLE_BLEND_ADDITIVE_2:
        blend_buf->m_src_factor = ui_runtime_render_src_color;
        blend_buf->m_dst_factor = ui_runtime_render_dst_color;
        break;
    case UI_PARTICLE_BLEND_HIGHLIGHT_1:
        blend_buf->m_src_factor = ui_runtime_render_src_color;
        blend_buf->m_dst_factor = ui_runtime_render_src_color;
        break;
    case UI_PARTICLE_BLEND_HIGHLIGHT_2:
        blend_buf->m_src_factor = ui_runtime_render_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_src_color;
        break;
    case UI_PARTICLE_BLEND_SUBDUEDLIGHT:
        blend_buf->m_src_factor = ui_runtime_render_one;
        blend_buf->m_dst_factor = ui_runtime_render_dst_color;
        break;
    case UI_PARTICLE_BLEND_ADD_PIC_LEVEL_1:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_src_color;
        blend_buf->m_dst_factor = ui_runtime_render_dst_color;
        break;
    case UI_PARTICLE_BLEND_ADD_PIC_LEVEL_2:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_dst_color;
        break;
    case UI_PARTICLE_BLEND_CG_EFFECTS:
        blend_buf->m_src_factor = ui_runtime_render_zero;
        blend_buf->m_dst_factor = ui_runtime_render_dst_color;
        break;
    case UI_PARTICLE_BLEND_MASK:
        blend_buf->m_src_factor = ui_runtime_render_zero;
        blend_buf->m_dst_factor = ui_runtime_render_src_alpha;
        break;
    case UI_PARTICLE_BLEND_RE_MASK:
        blend_buf->m_src_factor = ui_runtime_render_zero;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_alpha;
        break;
    case UI_PARTICLE_BLEND_RE_ALPHA_FILL:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_src_alpha;
        blend_buf->m_dst_factor = ui_runtime_render_src_alpha;
        break;
    case UI_PARTICLE_BLEND_HIGHLIGHT_PROTECT:
        blend_buf->m_src_factor = ui_runtime_render_src_color;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_color;
        break;
    case UI_PARTICLE_BLEND_HIGHLIGHT_COVER:
        blend_buf->m_src_factor = ui_runtime_render_src_color;
        blend_buf->m_dst_factor = ui_runtime_render_src_color;
        break;
    case UI_PARTICLE_BLEND_DARK_FLIP:
        blend_buf->m_src_factor = ui_runtime_render_src_color;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_dst_color;
        break;
    case UI_PARTICLE_BLEND_MIRROR_ADD:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_src_color;
        blend_buf->m_dst_factor = ui_runtime_render_one;
        break;
    case UI_PARTICLE_BLEND_HIGHLIGHT_ADD:
        blend_buf->m_src_factor = ui_runtime_render_src_color;
        blend_buf->m_dst_factor = ui_runtime_render_one;
        break;
    case UI_PARTICLE_BLEND_MINUS:
        blend_buf->m_src_factor = ui_runtime_render_dst_alpha;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_color;
        break;
    case UI_PARTICLE_BLEND_LINE_RE_ADD:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_color;
        break;
    case UI_PARTICLE_BLEND_RE_SUBDUEDLIGHT:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_dst_color;
        break;
    case UI_PARTICLE_BLEND_RE_ADD:
        blend_buf->m_src_factor = ui_runtime_render_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_dst_color;
        break;
    case UI_PARTICLE_BLEND_RE_DEEPED:
        blend_buf->m_src_factor = ui_runtime_render_zero;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_color;
        break;
    case UI_PARTICLE_BLEND_RE_FILTER_COLOR:
        blend_buf->m_src_factor = ui_runtime_render_dst_color;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_alpha;
        break;
    case UI_PARTICLE_BLEND_RE_LINE:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_src_alpha;
        blend_buf->m_dst_factor = ui_runtime_render_one_minus_src_alpha;
        break;
    case UI_PARTICLE_BLEND_EDGE_MAP:
        blend_buf->m_src_factor = ui_runtime_render_one_minus_dst_alpha;
        blend_buf->m_dst_factor = ui_runtime_render_dst_alpha;
        break;
    case UI_PARTICLE_BLEND_CUSTOM:
        CPE_ERROR(module->m_em, "plugin_particle_obj_render: emitter blend custom not support!");
        assert(0);
        blend_buf->m_src_factor = emitter_data->custom_src_factor;
        blend_buf->m_dst_factor = emitter_data->custom_des_factor;
        return NULL;
    default:
        CPE_ERROR(module->m_em, "plugin_particle_obj_render: emitter blend mode %d unknown!", emitter_data->blend_mode);
        return NULL;
    }

    assert(blend_buf->m_src_factor >= 0 && blend_buf->m_src_factor <= ui_runtime_render_zero);
    assert(blend_buf->m_dst_factor >= 0 && blend_buf->m_dst_factor <= ui_runtime_render_zero);
    return blend_buf;
}

static void plugin_particle_obj_calc_tiled_texture(
    ui_rect_t block_texture_rect,
    ui_rect_t texture_rect, uint32_t tiling_u, uint32_t tiling_v, uint32_t idx)
{
    ui_vector_2 texture_size = UI_VECTOR_2_INITLIZER(ui_rect_width(texture_rect), ui_rect_height(texture_rect));
    struct ui_vector_2 block_scale;
    uint32_t block_pos_x;
    uint32_t block_pos_y;
    ui_rect block_texture_adj;

    assert(tiling_u > 0);
    assert(tiling_v > 0);

    block_scale.x = 1.0f / (float)tiling_u;
    block_scale.y = 1.0f / (float)tiling_v;

    block_pos_y = floor((float)idx * block_scale.x);
    block_pos_x = idx - ((uint32_t)(block_pos_y * tiling_u));

    block_texture_adj.lt.x = block_pos_x * block_scale.x;
    block_texture_adj.lt.y = block_pos_y * block_scale.y;
    block_texture_adj.rb.x = block_texture_adj.lt.x + block_scale.x;
    block_texture_adj.rb.y = block_texture_adj.lt.y + block_scale.y;

    block_texture_rect->lt.x = texture_rect->lt.x + texture_size.x * block_texture_adj.lt.x;
    block_texture_rect->lt.y = texture_rect->lt.y + texture_size.y * block_texture_adj.lt.y;
    block_texture_rect->rb.x = texture_rect->lt.x + texture_size.x * block_texture_adj.rb.x;
    block_texture_rect->rb.y = texture_rect->lt.y + texture_size.y * block_texture_adj.rb.y;
}


void plugin_particle_obj_fill_rect(
    ui_runtime_render_t render,
    ui_transform_t transform, ui_runtime_render_second_color_t second_color,  ui_runtime_render_blend_t blend,
    ui_cache_res_t texture, ui_runtime_render_texture_filter_t filter,
    ui_rect_t target, ui_vector_2_t target_block_size,
    ui_rect_t texture_rect, ui_rect_t base_texture_rect)
{
    uint32_t abgr = ui_color_make_abgr(&second_color->m_color);
    
    //???
    assert(ui_rect_is_contain_pt(base_texture_rect, &texture_rect->lt));
    assert(ui_rect_width(texture_rect) <= ui_rect_width(base_texture_rect) + UI_FLOAT_PRECISION);
    assert(ui_rect_height(texture_rect) <= ui_rect_height(base_texture_rect) + UI_FLOAT_PRECISION);
    
    if (target_block_size == NULL) {
        int i;
        ui_vector_2 adj_buf[4] = {
            UI_VECTOR_2_INITLIZER(target->lt.x, target->lt.y)
            , UI_VECTOR_2_INITLIZER(target->rb.x, target->rb.y)
            , UI_VECTOR_2_INITLIZER(target->lt.x, target->rb.y)
            , UI_VECTOR_2_INITLIZER(target->rb.x, target->lt.y)
        };

        for(i = 0; i < CPE_ARRAY_SIZE(adj_buf); ++i) {
            ui_transform_inline_adj_vector_2(transform, adj_buf + i);
        }

        do {
            ui_runtime_vertex_v3f_t2f_c4ub v[4] = {
                { UI_VECTOR_3_INITLIZER(adj_buf[0].x, adj_buf[0].y, 0), UI_VECTOR_2_INITLIZER(texture_rect->lt.x, texture_rect->lt.y), abgr },
                { UI_VECTOR_3_INITLIZER(adj_buf[1].x, adj_buf[1].y, 0), UI_VECTOR_2_INITLIZER(texture_rect->rb.x, texture_rect->rb.y), abgr },
                { UI_VECTOR_3_INITLIZER(adj_buf[2].x, adj_buf[2].y, 0), UI_VECTOR_2_INITLIZER(texture_rect->lt.x, texture_rect->rb.y), abgr },
                { UI_VECTOR_3_INITLIZER(adj_buf[3].x, adj_buf[3].y, 0), UI_VECTOR_2_INITLIZER(texture_rect->rb.x, texture_rect->lt.y), abgr },
            };

            ui_runtime_render_cmd_quad_create_2d_buildin(
                render, 0.0f, NULL,
                v,
                texture, filter,
                ui_runtime_render_program_buildin_multiply, blend);
            
            /* ui_runtime_context_add_ha_rect( */
            /*     context, NULL, */
            /*     texture, v, */
            /*     ui_runtime_program_type_modulate, blend->filter, blend->m_src_factor, blend->m_dst_factor); */
        } while(0);
    }
    else {
        ui_vector_2 target_size = UI_VECTOR_2_INITLIZER(ui_rect_width(target), ui_rect_height(target));
        ui_vector_2 texture_size = UI_VECTOR_2_INITLIZER(ui_rect_width(texture_rect), ui_rect_height(texture_rect));
        ui_vector_2 texture_adj = UI_VECTOR_2_INITLIZER(0.75f / ui_cache_texture_width(texture), 0.75f / ui_cache_texture_height(texture));
        float process_width = target_size.x;
        float target_x = target->lt.x;

        /* printf("xxxx render: target=(%f,%f), target_block_size=(%f,%f)\n" */
        /*        "             texture_size=(%f,%f)\n" */
        /*        "             runtime_texture_size=(%d,%d)\n" */
        /*        , */
        /*        target_size.x, target_size.y, */
        /*        target_block_size->x, target_block_size->y, */
        /*        texture_size.x, texture_size.y, */
        /*        ui_cache_texture_width(texture), ui_cache_texture_height(texture)); */

        while(process_width > 0.0f) {
            float process_height = target_size.y;
            float target_y = target->lt.y;
            float texture_lt;
            float texture_rt;
            float cur_width;

            if (process_width == target_size.x) {
                texture_lt = texture_rect->lt.x;
                cur_width =
                    (texture_rect->rb.x > base_texture_rect->rb.x)
                    ? ((base_texture_rect->rb.x - texture_rect->lt.x) / texture_size.x) * target_block_size->x
                    : target_block_size->x;
            }
            else {
                texture_lt = base_texture_rect->lt.x;
                cur_width = target_block_size->x;
            }
            
            if (cur_width > process_width) {
                cur_width = process_width;
            }
            texture_rt = texture_lt + texture_size.x * cur_width / target_block_size->x;

            if (cpe_float_cmp(texture_lt, texture_rect->lt.x, UI_FLOAT_PRECISION) == 0) texture_lt += texture_adj.x;
            if (cpe_float_cmp(texture_rt, texture_rect->rb.x, UI_FLOAT_PRECISION) == 0) texture_rt -= texture_adj.x;
            
            while(process_height > 0.0f) {
                float cur_height;
                float texture_tp;
                float texture_bm;
                ui_vector_2 adj_buf[4];
                int i;

                if (process_height == target_size.y) {
                    texture_tp = texture_rect->lt.y;
                    cur_height =
                        (texture_rect->rb.y > base_texture_rect->rb.y)
                        ? ((base_texture_rect->rb.y - texture_rect->lt.y) / texture_size.y) * target_block_size->y
                        : target_block_size->y;
                }
                else {
                    texture_tp = base_texture_rect->lt.y;
                    cur_height = target_block_size->y;
                }

                if (cur_height > process_height) {
                    cur_height = process_height;
                }

                texture_bm = texture_tp + texture_size.y * cur_height / target_block_size->y;

                if (cpe_float_cmp(texture_tp, texture_rect->lt.y, UI_FLOAT_PRECISION) == 0) texture_tp += texture_adj.y;
                if (cpe_float_cmp(texture_bm, texture_rect->rb.y, UI_FLOAT_PRECISION) == 0) texture_bm -= texture_adj.y;
                
                adj_buf[0].x = target_x            ; adj_buf[0].y = target_y;
                adj_buf[1].x = target_x + cur_width; adj_buf[1].y = target_y + cur_height;
                adj_buf[2].x = target_x            ; adj_buf[2].y = target_y + cur_height;
                adj_buf[3].x = target_x + cur_width; adj_buf[3].y = target_y;

                for(i = 0; i < CPE_ARRAY_SIZE(adj_buf); ++i) {
                    ui_transform_inline_adj_vector_2(transform, adj_buf + i);
                }
                
                do {
                    ui_runtime_vertex_v3f_t2f_c4ub v[4] = {
                        { UI_VECTOR_3_INITLIZER(adj_buf[0].x, adj_buf[0].y, 0.0f), UI_VECTOR_2_INITLIZER(texture_lt, texture_tp), abgr },
                        { UI_VECTOR_3_INITLIZER(adj_buf[1].x, adj_buf[1].y, 0.0f), UI_VECTOR_2_INITLIZER(texture_rt, texture_bm), abgr },
                        { UI_VECTOR_3_INITLIZER(adj_buf[2].x, adj_buf[2].y, 0.0f), UI_VECTOR_2_INITLIZER(texture_lt, texture_bm), abgr },
                        { UI_VECTOR_3_INITLIZER(adj_buf[3].x, adj_buf[3].y, 0.0f), UI_VECTOR_2_INITLIZER(texture_rt, texture_tp), abgr },
                    };

                    /* printf("     (%f,%f) - (%f,%f) - (%f,%f) - (%f-%f)   from  (%f,%f) - (%f,%f) - (%f,%f) - (%f,%f)\n", */
                    /*        v[0].x, v[0].y, v[1].x, v[1].y, v[2].x, v[2].y, v[3].x, v[3].y, */
                    /*        v[0].u, v[0].v, v[1].u, v[1].v, v[2].u, v[2].v, v[3].u, v[3].v */
                    /*     ); */

                    ui_runtime_render_cmd_quad_create_2d_buildin(
                        render, 0.0f, NULL,
                        v,
                        texture, filter,
                        ui_runtime_render_program_buildin_multiply, blend);

                    /* ui_runtime_context_add_ha_rect( */
                    /*     render, NULL, */
                    /*     texture, v, */
                    /*     ui_runtime_program_type_modulate, blend->filter, blend->m_src_factor, blend->m_dst_factor); */
                } while(0);

                assert(cur_height > 0.0f);
                process_height -= cur_height;
                target_y += cur_height;
            }

            assert(cur_width > 0.0f);
            process_width -= cur_width;
            target_x += cur_width;
        }
    }
}
