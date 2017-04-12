#ifndef PLUGIN_UI_CONTROL_I_H
#define PLUGIN_UI_CONTROL_I_H
#include "cpe/utils/bitarry.h"
#include "render/utils/ui_color.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_vector_3.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_meta_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum plugin_ui_control_flag {
    plugin_ui_control_flag_visible = 0,
    plugin_ui_control_flag_enable = 1,
    plugin_ui_control_flag_draw_inner = 2,
    plugin_ui_control_flag_accept_ptls = 3,
    plugin_ui_control_flag_accept_szls = 4,
    plugin_ui_control_flag_accept_move = 5,
    plugin_ui_control_flag_accept_clip = 6,
    plugin_ui_control_flag_parent_clip = 7,
    plugin_ui_control_flag_accept_click = 8,
    plugin_ui_control_flag_size_zero = 9,
    plugin_ui_control_flag_disable_global_resize = 10,
    plugin_ui_control_flag_force_clicp = 11,
    plugin_ui_control_flag_frame_updated = 12,
    plugin_ui_control_flag_accept_click_move = 13,
    plugin_ui_control_flag_accept_double_click = 14,
    plugin_ui_control_flag_accept_float = 15,
    plugin_ui_control_flag_accept_global_disable_color = 16,
};
    
enum plugin_ui_control_cache_flag {
    plugin_ui_control_cache_flag_color = 0,    
    plugin_ui_control_cache_flag_scale = 1,    
    plugin_ui_control_cache_flag_angle = 2,
    plugin_ui_control_cache_flag_pos = 3,
    plugin_ui_control_cache_flag_pos_abs = 4,
    plugin_ui_control_cache_flag_size = 5,
    plugin_ui_control_cache_flag_layout = 6,
    plugin_ui_control_cache_flag_count = 7
};

struct plugin_ui_control {
    plugin_ui_control_meta_t m_meta;
    TAILQ_ENTRY(plugin_ui_control) m_next_for_meta;
    plugin_ui_page_t m_page;
    plugin_ui_control_t m_parent;
    TAILQ_ENTRY(plugin_ui_control) m_next_for_parent;
    plugin_ui_aspect_ref_list_t m_aspects;
    uint8_t m_is_processing;
    uint8_t m_is_free;
    uint16_t m_child_count;
    plugin_ui_control_list_t m_childs;
    plugin_ui_control_action_slots_t m_action_slots;
    plugin_ui_control_timer_list_t m_timers;
    plugin_ui_control_binding_list_t m_bindings;
    plugin_ui_control_frame_list_t m_frames;

    ui_data_control_t m_src;
    ui_data_control_t m_template;
    uint32_t m_flag;
    uint16_t m_cache_flag;
    uint16_t m_render_usables;
    uint16_t m_render_layers;
    
    char * m_name;
    char * m_user_text;

	uint8_t m_align_horz;
	uint8_t m_align_vert;

	/*clip*/
    ui_rect m_cliped_rt_abs;

    /*运行时真实的位置信息 */
    ui_color m_render_color;
    ui_vector_2 m_render_pt_to_p; /*相对父控件的坐标 */
    ui_vector_2 m_render_pt_abs; /*屏幕绝对坐标 */
    ui_vector_2 m_render_sz_abs; /*控件绝对大小 */
    ui_vector_2 m_render_sz_ns;  /*控件的没有scale的大小 */
    ui_vector_2 m_render_scale;
    ui_vector_3 m_render_angle;

    /*附加的绘制参数，用于调制表现层的绘制位置 */
    ui_vector_2 m_all_frame_pt;
    ui_vector_2 m_all_frame_scale;
    
    /*内部控件大小跟踪 */
    ui_vector_2 m_client_real_sz;
    ui_rect m_client_real_pd;

    /*相对于原始屏幕的原始值 */
    ui_vector_2 m_editor_pt;
    ui_vector_2 m_editor_sz;
    ui_rect m_editor_pd;

    /*缩放计算的输入位置信息 */
    UI_UNIT_VECTOR_2 m_render_pt;
    UI_UNIT_VECTOR_2 m_render_sz;

    /*屏幕内绘制区域，对Scroll等控件有效 */
    UI_UNIT_VECTOR_2 m_client_sz;
    UI_UNIT_RECT m_client_pd;

    /*滚动 */
    ui_vector_2 m_scroll_pt;

    ui_vector_2 m_pivot;
    ui_vector_3 m_angle;
    ui_vector_2 m_scale;
    ui_vector_2 m_float_scale;
    float m_alpha;
    ui_color m_color;
    ui_color m_gray_color;

    uint8_t m_draw_align;
    
    plugin_ui_animation_control_list_t m_animations;
};

void plugin_ui_control_cache_client(plugin_ui_control_t control, ui_vector_2_t client_sz);
uint8_t plugin_ui_control_cache_render_pt_abs(plugin_ui_control_t control);
void plugin_ui_control_cache_render_pt_abs_r(plugin_ui_control_t control);

void plugin_ui_control_render_tree(plugin_ui_control_t control, ui_runtime_render_t ctx, ui_rect_t rect);
int plugin_ui_control_do_update(plugin_ui_control_t control, void * ctx);

void plugin_ui_control_do_clear(plugin_ui_control_t control);
int plugin_ui_control_do_init(plugin_ui_page_t page, plugin_ui_control_t control, plugin_ui_control_meta_t meta);
void plugin_ui_control_do_fini(plugin_ui_control_t control);

void plugin_ui_control_real_free(plugin_ui_control_t control);

#ifdef __cplusplus
}
#endif

#endif
