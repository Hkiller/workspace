#include <assert.h>
#include "gameswf/gameswf_impl.h"
#include "base/image.h"
#include "base/utility.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_color.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_transform.h"
#include "render/cache/ui_cache_texture.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_cmd.h"
#include "plugin_swf_render_i.hpp"
#include "plugin_swf_bitmap_i.hpp"
#include "plugin_swf_video_i.hpp"

plugin_swf_render_handler::plugin_swf_render_handler(plugin_swf_module_t module)
    : m_module(module)
    , m_display_width(0)
    , m_display_height(0)
    , m_active_rgba(0)
    , m_mask_level(0)
{
    TAILQ_INIT(&m_bitmaps);
}

plugin_swf_render_handler::~plugin_swf_render_handler() {
    assert(TAILQ_EMPTY(&m_bitmaps));
}

void plugin_swf_render_handler::make_next_miplevel(int* width, int* height, Uint8* data) {
    assert(width);
    assert(height);
    assert(data);

    int	new_w = *width >> 1;
    int	new_h = *height >> 1;
    if (new_w < 1) new_w = 1;
    if (new_h < 1) new_h = 1;
		
    if (new_w * 2 != *width	 || new_h * 2 != *height)
    {
        // Image can't be shrunk along (at least) one
        // of its dimensions, so don't bother
        // resampling.	Technically we should, but
        // it's pretty useless at this point.  Just
        // change the image dimensions and leave the
        // existing pixels.
    }
    else
    {
        // Resample.  Simple average 2x2 --> 1, in-place.
        for (int j = 0; j < new_h; j++) {
            Uint8*	out = ((Uint8*) data) + j * new_w;
            Uint8*	in = ((Uint8*) data) + (j << 1) * *width;
            for (int i = 0; i < new_w; i++) {
                int	a;
                a = (*(in + 0) + *(in + 1) + *(in + 0 + *width) + *(in + 1 + *width));
                *(out) = a >> 2;
                out++;
                in += 2;
            }
        }
    }

    // Munge parameters to reflect the shrunken image.
    *width = new_w;
    *height = new_h;
}

bitmap_info* plugin_swf_render_handler::create_bitmap_info_rgb(image::rgb* im) {
    ui_cache_res_t texture = ui_cache_texture_create_with_buff(
        m_module->m_cache_mgr,  ui_cache_pf_r8g8b8, im->m_width, im->m_height, im->m_data, im->m_pitch * im->m_height);
    if (texture == NULL) {
        CPE_ERROR(m_module->m_em, "plugin_swf_render_handler: create texture (rgb) fail!");
        return NULL;
    }

    return new plugin_swf_bitmap(this, texture);
}

bitmap_info* plugin_swf_render_handler::create_bitmap_info_rgba(image::rgba* im) {
    ui_cache_res_t texture = ui_cache_texture_create_with_buff(
        m_module->m_cache_mgr,  ui_cache_pf_r8g8b8a8, im->m_width, im->m_height, 
        im->m_data, im->m_pitch * im->m_height);
    if (texture == NULL) {
        CPE_ERROR(m_module->m_em, "plugin_swf_render_handler: create texture (rgba) fail!");
        return NULL;
    }

    return new plugin_swf_bitmap(this, texture);
}

bitmap_info* plugin_swf_render_handler::create_bitmap_info_empty() {
    return new plugin_swf_bitmap(this, NULL);
}

bitmap_info* plugin_swf_render_handler::create_bitmap_info_alpha(int w, int h, Uint8* data) {
    ui_cache_res_t texture = ui_cache_texture_create_with_buff(m_module->m_cache_mgr, ui_cache_pf_a8, w, h,  data, w * h);
    if (texture == NULL) {
        CPE_ERROR(m_module->m_em, "plugin_swf_render_handler: create texture (alpha) fail!");
        return NULL;
    }
    
    return new plugin_swf_bitmap(this, texture);
}

video_handler* plugin_swf_render_handler::create_video_handler(void) {
    return new plugin_swf_video_handler(m_module);
}

void plugin_swf_render_handler::begin_display(
    gameswf::rgba background_color,
    int viewport_x0, int viewport_y0,
    int viewport_width, int viewport_height,
    float x0, float x1, float y0, float y1)
{
    m_display_width = fabsf(x1 - x0);
    m_display_height = fabsf(y1 - y0);
}

void plugin_swf_render_handler::end_display() {
}

void plugin_swf_render_handler::set_matrix(const gameswf::matrix& m) {
    //printf("xxxxx: set matrix: scale=(%f,%f) rotation=%f\n", m.get_x_scale(), m.get_y_scale(), m.get_rotation());
    m_current_matrix_swf = m;
    m_current_matrix = UI_MATRIX_4X4_ZERO;
    m_current_matrix.m[0] = m.m_[0][0];
    m_current_matrix.m[1] = m.m_[1][0];
    m_current_matrix.m[4] = m.m_[0][1];
    m_current_matrix.m[5] = m.m_[1][1];
    m_current_matrix.m[10] = 1.0f;
    m_current_matrix.m[12] = m.m_[0][2];
    m_current_matrix.m[13] = m.m_[1][2];
    m_current_matrix.m[15] = 1.0f;
}

void plugin_swf_render_handler::apply_color(const gameswf::rgba& c) {
    m_active_rgba = ui_color_make_abgr_from_value(c.m_r, c.m_g, c.m_b, c.m_a);
}

void plugin_swf_render_handler::fill_style_disable(int fill_side) {
    assert(fill_side >= 0 && fill_side < 2);
    m_current_styles[fill_side].disable();
}

void plugin_swf_render_handler::line_style_disable() {
    m_current_styles[LINE_STYLE].disable();
}

void plugin_swf_render_handler::fill_style_color(int fill_side, const gameswf::rgba& color) {
    assert(fill_side >= 0 && fill_side < 2);
    m_current_styles[fill_side].set_color(m_current_cxform.transform(color));
}

void plugin_swf_render_handler::line_style_color(gameswf::rgba color) {
    m_current_styles[LINE_STYLE].set_color(m_current_cxform.transform(color));
}

void plugin_swf_render_handler::fill_style_bitmap(
    int fill_side, gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm, bitmap_blend_mode bm)
{
    assert(fill_side >= 0 && fill_side < 2);
    m_current_styles[fill_side].set_bitmap(bi, m, wm, m_current_cxform);
}
	
void plugin_swf_render_handler::line_style_width(float width) {
    m_current_styles[LINE_STYLE].m_width = width;
}

void plugin_swf_render_handler::draw_mesh_primitive(int primitive_type, const void* i_coords, int vertex_count) {
//     const coord_component * coords = (const coord_component *)i_coords;
//     fill_style & style = m_current_styles[LEFT_STYLE];
//     ui_runtime_render_op_t render_op;
    
//     assert(style.m_mode != fill_style::INVALID);
    
//     if (style.m_mode == fill_style::COLOR) {
//         apply_color(style.m_color);
//         render_op = create_render_op(primitive_type, ui_runtime_render_program_type_modulate);
//     }
//     else if (style.m_mode == fill_style::BITMAP_WRAP || style.m_mode == fill_style::BITMAP_CLAMP) {
//         assert(style.m_bitmap_info != NULL);

//         apply_color(style.m_color);
//         // Do the modulate part of the color
//         // transform in the first pass.  The
//         // additive part, if any, needs to
//         // happen in a second pass.
//         // glColor4f(m_bitmap_color_transform.m_[0][0],
//         //           m_bitmap_color_transform.m_[1][0],
//         //           m_bitmap_color_transform.m_[2][0],
//         //           m_bitmap_color_transform.m_[3][0]
//         //     );
//         render_op = create_render_op(primitive_type, ui_runtime_render_program_type_modulate);

//         style.m_bitmap_info->layout();
        
//         if (style.m_mode == fill_style::BITMAP_CLAMP) {
//             ui_runtime_render_op_set_texture(render_op, 0, style.m_bitmap_info->m_texture, GL_CLAMP_TO_EDGE, GL_LINEAR);
//         }
//         else {
//             assert(style.m_mode == fill_style::BITMAP_WRAP);
//             ui_runtime_render_op_set_texture(render_op, 0, style.m_bitmap_info->m_texture, GL_REPEAT, GL_LINEAR);
//         }
//     }
//     else {
//         render_op = create_render_op(primitive_type, ui_runtime_render_program_type_modulate);
//     }
//     if (render_op == NULL) return;

//     // ui_matrix_4x4 mat;
//     // ui_matrix_4x4_cross_product(&mat, &m_current_matrix, ui_transform_calc_matrix_4x4(m_current_context.m_transform));
    
//     ui_runtime_vertex_t vertexs = ui_runtime_vertex_buff_alloc(render_op, vertex_count);

//     for(int i = 0; i < vertex_count; ++i) {
//         ui_runtime_vertex_t v = vertexs + i;

//         gameswf::point pt_i(coords[i * 2], coords[i * 2 + 1]);

//         gameswf::point pt_w;
//         m_current_matrix_swf.transform(&pt_w, pt_i);

//         ui_vector_2 pt_final = UI_VECTOR_2_INITLIZER(pt_w.m_x, pt_w.m_y);
//         ui_transform_inline_adj_vector_2(m_current_context.m_transform, &pt_final);

//         ui_vector_2 pt_i2 = UI_VECTOR_2_INITLIZER(coords[i * 2], coords[i * 2 + 1]);
//         ui_vector_2 pt_w2;
//         ui_matrix_4x4_adj_vector_2(&m_current_matrix, &pt_w2, &pt_i2);
        
//         printf("    (%f,%f) ==> swf=(%f,%f) (%f,%f), final=(%f,%f)\n", pt_i.m_x, pt_i.m_y, pt_w.m_x, pt_w.m_y, pt_w2.x, pt_w2.y, pt_final.x, pt_final.y);
//         v->x = pt_final.x;
//         v->y = pt_final.y;
//         v->z = 0.0f;
//         v->u = 0.0f;
//         v->v = 0.0f;
//         v->c = m_active_rgba;
//     }
    
//     if (style.m_mode == fill_style::BITMAP_WRAP || style.m_mode == fill_style::BITMAP_CLAMP) {
//         // Set up the bitmap matrix for texgen.
//         float	inv_width = 1.0f / style.m_bitmap_info->get_width();
//         float	inv_height = 1.0f / style.m_bitmap_info->get_height();

//         const gameswf::matrix&	m = style.m_bitmap_matrix;

//         float pS[4];
//         pS[0] = m.m_[0][0] * inv_width;
//         pS[1] = m.m_[0][1] * inv_width;
//         pS[2] = 0;
//         pS[3] = m.m_[0][2] * inv_width;
        
//         float pT[4];
//         pT[0] = m.m_[1][0] * inv_height;
//         pT[1] = m.m_[1][1] * inv_height;
//         pT[2] = 0;
//         pT[3] = m.m_[1][2] * inv_height;
        

//         for(int i = 0; i < vertex_count; ++i) {
//             ui_runtime_vertex_t v = vertexs + i;
//             v->u = coords[i] * pS[0] + coords[i+1] * pS[1] + pS[3];
//             v->v = coords[i] * pT[0] + coords[i+1] * pT[1] + pT[3];
//         }
//     }

//     switch(primitive_type) {
//     case GL_TRIANGLE_STRIP:
//         if (ui_runtime_index_buff_build_for_triangle_strip(render_op, vertex_count) != 0) goto BUILD_OP_ERROR;
//         break;
//     case GL_TRIANGLES:
//         if (ui_runtime_index_buff_build_for_triangles(render_op, vertex_count) != 0) goto BUILD_OP_ERROR;
//         break;
//     default:
//         assert(false);
//     }
    
//     assert(!m_current_styles[LEFT_STYLE].needs_second_pass());
		
//     ui_runtime_render_op_create_done(render_op);
//     return;

// BUILD_OP_ERROR:
//     ui_runtime_render_op_create_cancel(render_op);
}

void plugin_swf_render_handler::draw_mesh_strip(const void* coords, int vertex_count) {
    //draw_mesh_primitive(GL_TRIANGLE_STRIP, coords, vertex_count);
}
			
void plugin_swf_render_handler::draw_triangle_list(const void* coords, int vertex_count) {
    //draw_mesh_primitive(GL_TRIANGLES, coords, vertex_count);
}

void plugin_swf_render_handler::draw_line_strip(const void* i_coords, int vertex_count) {
    const coord_component * coords = (const coord_component *)i_coords;
//     m_current_styles[LINE_STYLE].apply();

//     float scale = fabsf(m_current_matrix.get_x_scale()) + fabsf(m_current_matrix.get_y_scale());
//     float w = m_current_styles[LINE_STYLE].m_width * scale / 2.0f;
//     w = TWIPS_TO_PIXELS(w);

//     glLineWidth(w <= 1.0f ? 1.0f : w);

//     glMatrixMode(GL_MODELVIEW);
//     glPushMatrix();
//     apply_matrix(m_current_matrix);

//     // Send the line-strip to OpenGL
//     glEnableClientState(GL_VERTEX_ARRAY);
// #if TU_USES_FLOAT_AS_COORDINATE_COMPONENT
//     glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, coords);
// #else
//     glVertexPointer(2, GL_SHORT, sizeof(Sint16) * 2, coords);
// #endif
		
//     glEnable(GL_LINE_SMOOTH);
//     glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
//     glDrawArrays(GL_LINE_STRIP, 0, vertex_count);
//     glDisable(GL_LINE_SMOOTH);
		
//     // Draw a round dot on the beginning and end coordinates to lines.
//     glPointSize(w);
//     glEnable(GL_POINT_SMOOTH);
//     glDrawArrays(GL_POINTS, 0, vertex_count);
//     glDisable(GL_POINT_SMOOTH);
//     glPointSize(1);

//     glDisableClientState(GL_VERTEX_ARRAY);

//     // restore defaults
//     glPointSize(1);
//     glLineWidth(1);

//     glPopMatrix();
}

void plugin_swf_render_handler::draw_bitmap(
    const gameswf::matrix& m,
    gameswf::bitmap_info* i_bi,
    const gameswf::rect& coords,
    const gameswf::rect& uv_coords,
    gameswf::rgba color)
{
    // plugin_swf_bitmap * bi = dynamic_cast<plugin_swf_bitmap *>(i_bi);
    // gameswf::point a, b, c, d;

    // assert(bi);
    // bi->layout();

    // apply_color(color);

    // m.transform(&a, gameswf::point(coords.m_x_min, coords.m_y_min));
    // m.transform(&b, gameswf::point(coords.m_x_max, coords.m_y_min));
    // m.transform(&c, gameswf::point(coords.m_x_min, coords.m_y_max));
    // d.m_x = b.m_x + c.m_x - a.m_x;
    // d.m_y = b.m_y + c.m_y - a.m_y;

    // ui_runtime_render_op_t render_op = create_render_op(GL_TRIANGLES, ui_runtime_render_program_type_modulate);
    // if (render_op == NULL) return;

    // ui_runtime_vertex v[4] = {
    //     { a.m_x, a.m_y, 0, uv_coords.m_x_min, uv_coords.m_y_min, m_active_rgba },
    //     { b.m_x, b.m_y, 0, uv_coords.m_x_max, uv_coords.m_y_min, m_active_rgba },
    //     { c.m_x, c.m_y, 0, uv_coords.m_x_min, uv_coords.m_y_max, m_active_rgba },
    //     { d.m_x, d.m_y, 0, uv_coords.m_x_max, uv_coords.m_y_max, m_active_rgba },
    // };

    // if ((bi->m_texture && ui_runtime_render_op_set_texture(render_op, 0, bi->m_texture, GL_CLAMP_TO_EDGE, GL_LINEAR) != 0)
    //     || ui_runtime_render_add_vectex_to_op(m_current_context.m_context, bi->m_texture, render_op, v, 4) != 0
    //     || ui_runtime_index_buff_build_for_rect(render_op) != 0
    //     )
    // {
    //     ui_runtime_render_op_create_cancel(render_op);
    //     return;
    // }

    // ui_runtime_render_op_alphablend_enable(render_op, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ui_runtime_render_op_create_done(render_op);
}
	
bool plugin_swf_render_handler::test_stencil_buffer(const gameswf::rect& bound, Uint8 pattern) {
    // get viewport size
    // GLint vp[4]; 
    // glGetIntegerv(GL_VIEWPORT, vp); 
    // int vp_width = vp[2];
    // int vp_height = vp[3];

    bool ret = false;

    // int x0 = (int) bound.m_x_min;
    // int y0 = (int) bound.m_y_min;
    // int width = (int) bound.m_x_max - x0;
    // int height = (int) bound.m_y_max - y0;

    // if (width > 0 && height > 0 &&
    //     x0 >= 0 && x0 + width <= vp_width &&
    //     y0 >= 0 && y0 + height <= vp_height)
    // {
    //     int bufsize = width * height;

    //     mem_buffer_t tmp_buffer = gd_app_tmp_buffer(m_module->m_app);
    //     mem_buffer_clear_data(tmp_buffer);
    //     uint8_t buf = mem_buffer_alloc(tmp_buffer, 4 * bufsize);

    //     //glReadPixels(x0, vp[3] - y0 - height, width, height, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buf);

    //     for (int i = 0; i < bufsize; i++) {
    //         if (buf[i] == pattern) {
    //             ret = true;
    //             break;
    //         }
    //     }
    // }

    return ret;
}

void plugin_swf_render_handler::begin_submit_mask() {
    if (m_mask_level == 0) {
        // assert(glIsEnabled(GL_STENCIL_TEST) == false);
        // glEnable(GL_STENCIL_TEST);
        // glClearStencil(0);
        // glClear(GL_STENCIL_BUFFER_BIT);
    }

    // disable framebuffer writes
    //glColorMask(0, 0, 0, 0);

    // we set the stencil buffer to 'm_mask_level+1' 
    // where we draw any polygon and stencil buffer is 'm_mask_level'
    // glStencilFunc(GL_EQUAL, m_mask_level++, 0xFF);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); 
}

void plugin_swf_render_handler::end_submit_mask() {	     
    // // enable framebuffer writes
    // glColorMask(1, 1, 1, 1);

    // // we draw only where the stencil is m_mask_level (where the current mask was drawn)
    // glStencilFunc(GL_EQUAL, m_mask_level, 0xFF);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	
}

void plugin_swf_render_handler::disable_mask() {	     
//     assert(m_mask_level > 0);
//     if (--m_mask_level == 0)
//     {
//         glDisable(GL_STENCIL_TEST); 
//         return;
//     }

//     // begin submit previous mask

//     glColorMask(0, 0, 0, 0);

//     // we set the stencil buffer to 'm_mask_level' 
//     // where the stencil buffer m_mask_level + 1
//     glStencilFunc(GL_EQUAL, m_mask_level + 1, 0xFF);
//     glStencilOp(GL_KEEP, GL_KEEP, GL_DECR); 

//     // draw the quad to fill stencil buffer

// //		glBegin(GL_QUADS);
// //		glVertex2f(0, 0);
// //		glVertex2f(m_display_width, 0);
// //		glVertex2f(m_display_width, m_display_height);
// //		glVertex2f(0, m_display_height);
// //		glEnd();
		
//     Sint16 squareVertices[8]; 
//     squareVertices[0] = 0;
//     squareVertices[1] = 0;
//     squareVertices[2] = m_display_width;
//     squareVertices[3] = 0;
//     squareVertices[4] = 0;
//     squareVertices[5] = m_display_height;
//     squareVertices[6] = m_display_width;
//     squareVertices[7] = m_display_height;
		
//     glVertexPointer(2, GL_SHORT, 0, squareVertices);
//     glEnableClientState(GL_VERTEX_ARRAY);
//     glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

//     glDisableClientState(GL_VERTEX_ARRAY);

//     end_submit_mask();
}

bool plugin_swf_render_handler::is_visible(const gameswf::rect& bound) {
    gameswf::rect viewport;
    viewport.m_x_min = 0;
    viewport.m_y_min = 0;
    viewport.m_x_max = m_display_width;
    viewport.m_y_max = m_display_height;
    return viewport.bound_test(bound);
}

// ui_runtime_render_op_t
// plugin_swf_render_handler::create_render_op(int mode, ui_runtime_render_program_type_t program_type) {
//     ui_runtime_render_op_buff_t op_buffer = ui_runtime_render_op_buff(m_current_context.m_context);

//     ui_runtime_render_program_t program = ui_runtime_module_default_program(m_module->m_runtime, program_type);
//     if (program == NULL) {
//         CPE_ERROR(m_module->m_em, "plugin_swf_render: program %d not exist", program_type);
//         return NULL;
//     }
    
//     ui_runtime_render_op_t op = ui_runtime_render_op_create_begin(op_buffer, mode, program);
//     if (op == NULL) {
//         CPE_ERROR(m_module->m_em, "plugin_swf_render: begin op fail");
//         return NULL;
//     }

//     return op;
// }

plugin_swf_render_handler::fill_style::fill_style()
    : m_mode(INVALID)
    , m_has_nonzero_bitmap_additive_color(false)
{
}

bool plugin_swf_render_handler::fill_style::needs_second_pass() const {
    if (m_mode == BITMAP_WRAP || m_mode == BITMAP_CLAMP) {
        return m_has_nonzero_bitmap_additive_color;
    }
    else {
        return false;
    }
}

void plugin_swf_render_handler::fill_style::apply_second_pass() const {
    assert(needs_second_pass());

    // The additive color also seems to be modulated by the texture. So,
    // maybe we can fake this in one pass using using the mean value of 
    // the colors: c0*t+c1*t = ((c0+c1)/2) * t*2
    // I don't know what the alpha component of the color is for.
    //glDisable(GL_TEXTURE_2D);

    // glColor4f(
    //     m_bitmap_color_transform.m_[0][1] / 255.0f,
    //     m_bitmap_color_transform.m_[1][1] / 255.0f,
    //     m_bitmap_color_transform.m_[2][1] / 255.0f,
    //     m_bitmap_color_transform.m_[3][1] / 255.0f
    //     );

    // glBlendFunc(GL_ONE, GL_ONE);
}

void plugin_swf_render_handler::fill_style::cleanup_second_pass() const {
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void plugin_swf_render_handler::fill_style::set_bitmap(
    gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm, const gameswf::cxform& color_transform)
{
    m_mode = (wm == WRAP_REPEAT) ? BITMAP_WRAP : BITMAP_CLAMP;
    m_bitmap_info = dynamic_cast<plugin_swf_bitmap *>(bi);
    m_bitmap_matrix = m;
    m_bitmap_color_transform = color_transform;
    m_bitmap_color_transform.clamp();

    m_color = gameswf::rgba(
        Uint8(m_bitmap_color_transform.m_[0][0] * 255.0f), 
        Uint8(m_bitmap_color_transform.m_[1][0] * 255.0f), 
        Uint8(m_bitmap_color_transform.m_[2][0] * 255.0f), 
        Uint8(m_bitmap_color_transform.m_[3][0] * 255.0f));

    if (m_bitmap_color_transform.m_[0][1] > 1.0f
        || m_bitmap_color_transform.m_[1][1] > 1.0f
        || m_bitmap_color_transform.m_[2][1] > 1.0f
        || m_bitmap_color_transform.m_[3][1] > 1.0f)
    {
        m_has_nonzero_bitmap_additive_color = true;
    }
    else
    {
        m_has_nonzero_bitmap_additive_color = false;
    }
}
