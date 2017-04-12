#ifndef PLUGIN_UI_CONTROL_FRAME_I_H
#define PLUGIN_UI_CONTROL_FRAME_I_H
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin_ui_control_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_frame {
    plugin_ui_control_t m_control;
    TAILQ_ENTRY(plugin_ui_control_frame) m_next;
    plugin_ui_aspect_ref_list_t m_aspects;
    plugin_ui_control_frame_layer_t m_layer;
    plugin_ui_control_frame_usage_t m_usage;
    ui_runtime_render_obj_ref_t m_render_obj_ref;
    char * m_name;
    uint8_t m_auto_remove;
    uint8_t m_base_pos;
    uint8_t m_sync_transform;
    uint8_t m_sync_size;
    uint8_t m_draw_inner;
    ui_vector_2 m_base_size;     /*绘制元素的原始尺寸 */
    ui_vector_2 m_runtime_size;  /*绘制元素的实际尺寸（控制绘制对象的缩放由控件控制) */
    ui_vector_2 m_offset;        /*frame正对左上角点的控件内本地坐标 */
    ui_vector_2 m_scale;         /*绘制对象的额外缩放, 一般由脚本控制 */
    ui_vector_2 m_render_size;   /*绘制的尺寸，控件内本地坐标 */
    float m_priority;
    float m_alpha;
};

void plugin_ui_control_frame_real_free(plugin_ui_control_frame_t frame);

plugin_ui_control_frame_t plugin_ui_control_frame_find_touchable(plugin_ui_control_t control);

uint8_t plugin_ui_control_frame_touch_is_support(plugin_ui_control_frame_t frame);
int plugin_ui_control_frame_touch_dispatch(plugin_ui_control_frame_t frame, uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t pt);
    
void plugin_ui_control_do_render_frame(
    plugin_ui_control_frame_t frame, ui_color_t color, ui_rect_t render_rect,
    ui_runtime_render_t ctx, ui_rect_t screen_rect);


#ifdef __cplusplus
}
#endif

#endif
