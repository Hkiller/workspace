#include <assert.h>
#include "cpe/pal/pal_math.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_rect.h"
#include "render/cache/ui_cache_texture.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "render/runtime/ui_runtime_render_cmd_utils_2d.h"
#include "plugin_basicanim_utils_i.h"

ui_runtime_render_cmd_t
plugin_basicanim_render_draw_color(
    ui_runtime_render_t render, float logic_z,
    ui_rect_t clip_rect, ui_transform_t transform,
    ui_rect_t rect, ui_color_t color)
{
    ui_transform_t mvp;
    ui_runtime_render_cmd_t cmd;
    uint32_t abgr = ui_color_make_abgr(color);
    size_t i;
    ui_vector_2 points[] = {
        UI_VECTOR_2_INITLIZER(rect->lt.x, rect->lt.y),
        UI_VECTOR_2_INITLIZER(rect->rb.x, rect->rb.y),
        UI_VECTOR_2_INITLIZER(rect->lt.x, rect->rb.y),
        UI_VECTOR_2_INITLIZER(rect->rb.x, rect->lt.y)
    };

	/* 计算剪裁  */
	if (clip_rect) {
        mvp = NULL;
        
        for(i = 0; i < CPE_ARRAY_SIZE(points); ++i) {
            ui_transform_inline_adj_vector_2(transform, &points[i]);
        }

        for(i = 0; i < CPE_ARRAY_SIZE(points); ++i) {
            ui_vector_2_t pt = &points[i];
            
            if		(pt->x < clip_rect->lt.x) pt->x = clip_rect->lt.x;
            else if (pt->x > clip_rect->rb.x) pt->x = clip_rect->rb.x;
            if		(pt->y < clip_rect->lt.y) pt->y = clip_rect->lt.y;
            else if (pt->y > clip_rect->rb.y) pt->y = clip_rect->rb.y;
        }

        /* 完全剪裁 */
        if (points[2].y == points[0].y || points[3].x == points[0].x) {
            return NULL;
        }

        //printf("xxxxx: rect=(%f,%f)-(%f,%f)\n", points[0].x, points[0].y, points[1].x, points[1].y);
	}
    else {
        mvp = transform;
    }

    /*开始绘制 */
    do {
        struct ui_runtime_render_blend blend = { ui_runtime_render_src_alpha, ui_runtime_render_one_minus_src_alpha };
        ui_runtime_vertex_v3f_t2f_c4ub vertexs[4] = {
            { UI_VECTOR_3_INITLIZER(points[0].x, points[0].y, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), abgr },
            { UI_VECTOR_3_INITLIZER(points[1].x, points[1].y, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), abgr },
            { UI_VECTOR_3_INITLIZER(points[2].x, points[2].y, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), abgr },
            { UI_VECTOR_3_INITLIZER(points[3].x, points[3].y, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), abgr },
        };

        cmd = ui_runtime_render_cmd_quad_create_2d_buildin(
            render, 0.0f, mvp,
            vertexs,
            NULL, ui_runtime_render_filter_linear,
            ui_runtime_render_program_buildin_color,
            &blend);
    } while(0);

    return cmd;
}

void plugin_basicanim_render_draw_rect(
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd, ui_rect_t clip_rect,
    ui_transform_t transform, ui_runtime_render_second_color_t second_color,
    ui_cache_res_t texture, ui_rect_t texture_rect, ui_runtime_render_texture_filter_t texture_filter)
{
    ui_transform_t mvp;
	float x = texture_rect->lt.x;
	float y = texture_rect->lt.y;
	float w = texture_rect->rb.x - texture_rect->lt.x;
	float h = texture_rect->rb.y - texture_rect->lt.y;

    ui_vector_2 pt1 = UI_VECTOR_2_INITLIZER(0.0f, 0.0f);
    ui_vector_2 pt2 = UI_VECTOR_2_INITLIZER(w, h);
    ui_vector_2 pt3 = UI_VECTOR_2_INITLIZER(0.0f, h);
    ui_vector_2 pt4 = UI_VECTOR_2_INITLIZER(w, 0.0f);

	ui_vector_2 uv1;
	ui_vector_2 uv2;
	ui_vector_2 uv3;
	ui_vector_2 uv4;

	float invw;
	float invh;

	uint32_t abgr;
    float adj_x;
    float adj_y;
    
    invw = 1.0f / ui_cache_texture_width(texture);
	invh = 1.0f / ui_cache_texture_height(texture);

    adj_x = 0.5f;
    adj_y = 0.5f;
    
	// 计算剪裁 
	if (clip_rect && !transform->m_r_p) {
        ui_vector_2 tp1;
        ui_vector_2 tp2;
        ui_vector_2 tp3;
        ui_vector_2 tp4;
		float sx;
		float sy;
        ui_vector_2 tv1;
        ui_vector_2 tv2;
        ui_vector_2 tv3;
        ui_vector_2 tv4;

        ui_transform_inline_adj_vector_2(transform, &pt1);
        ui_transform_inline_adj_vector_2(transform, &pt2);
        ui_transform_inline_adj_vector_2(transform, &pt3);
        ui_transform_inline_adj_vector_2(transform, &pt4);
        mvp = NULL;

        tp1 = pt1;
        tp2 = pt2;
        tp3 = pt3;
        tp4 = pt4;
        
        if		(tp1.x < (float)clip_rect->lt.x)	pt1.x = (float)clip_rect->lt.x;
        else if (tp1.x > (float)clip_rect->rb.x)	pt1.x = (float)clip_rect->rb.x;
        if		(tp1.y < (float)clip_rect->lt.y)	pt1.y = (float)clip_rect->lt.y;
        else if (tp1.y > (float)clip_rect->rb.y)	pt1.y = (float)clip_rect->rb.y;

        if		(tp2.x < (float)clip_rect->lt.x)	pt2.x = (float)clip_rect->lt.x;
        else if (tp2.x > (float)clip_rect->rb.x)	pt2.x = (float)clip_rect->rb.x;
        if		(tp2.y < (float)clip_rect->lt.y)	pt2.y = (float)clip_rect->lt.y;
        else if (tp2.y > (float)clip_rect->rb.y)	pt2.y = (float)clip_rect->rb.y;

        if		(tp3.x < (float)clip_rect->lt.x)	pt3.x = (float)clip_rect->lt.x;
        else if (tp3.x > (float)clip_rect->rb.x)	pt3.x = (float)clip_rect->rb.x;
        if		(tp3.y < (float)clip_rect->lt.y)	pt3.y = (float)clip_rect->lt.y;
        else if (tp3.y > (float)clip_rect->rb.y)	pt3.y = (float)clip_rect->rb.y;

        if		(tp4.x < (float)clip_rect->lt.x)	pt4.x = (float)clip_rect->lt.x;
        else if (tp4.x > (float)clip_rect->rb.x)	pt4.x = (float)clip_rect->rb.x;
        if		(tp4.y < (float)clip_rect->lt.y)	pt4.y = (float)clip_rect->lt.y;
        else if (tp4.y > (float)clip_rect->rb.y)	pt4.y = (float)clip_rect->rb.y;

		/*完全剪裁 */
		if (pt3.y == pt1.y || pt4.x == pt1.x) return;

		/*剪裁默认是没有旋转变换的 
          否则必须用RenderTarget 
          所以我们直接拿矩阵元素作为缩放分量 */
		sx = transform->m_s.x;
		sy = transform->m_s.y;

		tv1.x = fabs((pt1.x-tp1.x)/sx);
        tv1.y = fabs((pt1.y-tp1.y)/sy);
        tv2.x = fabs((pt2.x-tp2.x)/sx);
        tv2.y = fabs((pt2.y-tp2.y)/sy);
        tv3.x = fabs((pt3.x-tp3.x)/sx);
        tv3.y = fabs((pt3.y-tp3.y)/sy);
        tv4.x = fabs((pt4.x-tp4.x)/sx);
        tv4.y = fabs((pt4.y-tp4.y)/sy);

		/*计算剪裁UV */
		uv1.x = (x  +tv1.x+adj_x) * invw;
        uv1.y = (y  +tv1.y+adj_y) * invh;
        uv2.x = (x+w-tv2.x-adj_x) * invw;
        uv2.y = (y+h-tv2.y-adj_y) * invh;
        uv3.x = (x  +tv3.x+adj_x) * invw;
        uv3.y = (y+h-tv3.y-adj_y) * invh;
        uv4.x = (x+w-tv4.x-adj_x) * invw;
        uv4.y = (y  +tv4.y+adj_y) * invh;
    }
	else {
        mvp = transform;

        // 0.375-> 0.5
		uv1.x = (x  +adj_x) * invw;
        uv1.y = (y  +adj_y) * invh;
		uv2.x = (x+w-adj_x) * invw;
        uv2.y = (y+h-adj_y) * invh;
		uv3.x = (x  +adj_x) * invw;
        uv3.y = (y+h-adj_y) * invh;
		uv4.x = (x+w-adj_x) * invw;
        uv4.y = (y  +adj_y) * invh;
	}

    assert(second_color);
    abgr = ui_color_make_abgr(&second_color->m_color);
    
    do {
        struct ui_runtime_render_blend blend = { ui_runtime_render_src_alpha, ui_runtime_render_one_minus_src_alpha };
        ui_runtime_vertex_v3f_t2f_c4ub vertexs[4] = {
            { UI_VECTOR_3_INITLIZER(pt1.x, pt1.y, 0.0f), UI_VECTOR_2_INITLIZER(uv1.u, uv1.v), abgr },
            { UI_VECTOR_3_INITLIZER(pt2.x, pt2.y, 0.0f), UI_VECTOR_2_INITLIZER(uv2.u, uv2.v), abgr },
            { UI_VECTOR_3_INITLIZER(pt3.x, pt3.y, 0.0f), UI_VECTOR_2_INITLIZER(uv3.u, uv3.v), abgr },
            { UI_VECTOR_3_INITLIZER(pt4.x, pt4.y, 0.0f), UI_VECTOR_2_INITLIZER(uv4.u, uv4.v), abgr },
        };

        if (batch_cmd) {
            ui_runtime_render_cmd_quad_batch_append(
                batch_cmd, render, 0.0f, mvp, vertexs,
                texture, texture_filter,
                ui_runtime_render_second_color_mix_to_program(second_color->m_mix),
                &blend);
        }
        else {
            ui_runtime_render_cmd_quad_create_2d_buildin(
                render, 0.0f, mvp,
                vertexs,
                texture, texture_filter,
                ui_runtime_render_second_color_mix_to_program(second_color->m_mix),
                &blend);
        }
    } while(0);
}
