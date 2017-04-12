#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "plugin/basicanim/plugin_basicanim_utils.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_page_i.h"

void plugin_ui_control_render_tree(plugin_ui_control_t control, ui_runtime_render_t ctx, ui_rect_t clip_rect) {
    plugin_ui_control_t child;
    uint8_t have_scissor = 0;
    ui_rect basic_rect;
    ui_rect inner_rect;
    ui_rect child_clip_rect;
    plugin_ui_control_frame_t frame;

    if (cpe_ba_get(&control->m_flag, plugin_ui_control_flag_size_zero)) return;

    if (control->m_cache_flag) {
        plugin_ui_control_update_cache(control, 0);
    }

    /*计算绘制区域 */
    basic_rect = plugin_ui_control_real_rt_abs(control);
    if (!ui_rect_is_valid(&basic_rect)) return;
    ui_assert_rect_sane(&basic_rect);

    ui_rect_intersection(&child_clip_rect, &basic_rect, clip_rect);
    if (!ui_rect_is_valid(&child_clip_rect)) return;

    inner_rect = plugin_ui_control_real_inner_abs(control);
    
    if (plugin_ui_control_force_clicp(control)) {
        have_scissor = ui_runtime_render_scissor_push(ctx, &basic_rect) == 0 ? 1 : 0;
    }

    /*绘制背景 */
    for(frame = TAILQ_FIRST(&control->m_frames); frame; frame = TAILQ_NEXT(frame, m_next)) {
        if (frame->m_layer >= plugin_ui_control_frame_layer_text) break;
        if (!plugin_ui_control_frame_visible(frame)) continue;
        if (!cpe_ba_get(&control->m_render_usables, frame->m_usage)) continue;
        if (!cpe_ba_get(&control->m_render_layers, frame->m_layer)) continue;
        
        plugin_ui_control_do_render_frame(frame, NULL, frame->m_draw_inner ? &inner_rect : &basic_rect, ctx, clip_rect);
    }

    /*绘制子节点 */
    TAILQ_FOREACH_REVERSE(child, &control->m_childs, plugin_ui_control_list, m_next_for_parent) {
        if (!cpe_ba_get(&child->m_flag, plugin_ui_control_flag_visible)) continue;
        if (cpe_ba_get(&child->m_flag, plugin_ui_control_flag_size_zero)) continue;
        if (!ui_rect_is_valid(&child->m_cliped_rt_abs)) continue;
        
        plugin_ui_control_render_tree(child, ctx, &child_clip_rect);
    }

    /*绘制前景 */
    for(; frame; frame = TAILQ_NEXT(frame, m_next)) {
        if (!plugin_ui_control_frame_visible(frame)) continue;
        if (!cpe_ba_get(&control->m_render_usables, frame->m_usage)) continue;
        if (!cpe_ba_get(&control->m_render_layers, frame->m_layer)) continue;
        
        plugin_ui_control_do_render_frame(frame, NULL, frame->m_draw_inner ? &inner_rect : &basic_rect, ctx, clip_rect);
    }

    if (have_scissor) {
        ui_runtime_render_scissor_pop(ctx);
    }
}

void plugin_ui_env_render(plugin_ui_env_t env, ui_runtime_render_t ctx) {
    plugin_ui_page_t page;
    struct ui_rect render_rect = UI_RECT_INITLIZER(0.0f, 0.0f, env->m_runtime_sz.x, env->m_runtime_sz.y);

    ui_runtime_render_set_active_camera(ctx, env->m_camera);
    
    TAILQ_FOREACH(page, &env->m_visible_pages, m_next_for_visible_queue) {
        
        /*更新现实数据 */
        if (page->m_control.m_cache_flag) {
            plugin_ui_control_update_cache(&page->m_control, 0);
        }

        /* printf( */
        /*     "render %s.%s: page %s\n", */
        /*     plugin_ui_phase_name(plugin_ui_phase_node_current_phase(cur_phase)), */
        /*     plugin_ui_state_name(plugin_ui_state_node_current_state(state_node)), */
        /*     plugin_ui_page_name(page)); */

        plugin_ui_control_render_tree(&page->m_control, ctx, &render_rect);
    }
}
