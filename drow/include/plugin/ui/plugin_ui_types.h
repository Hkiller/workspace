#ifndef DROW_PLUGIN_UI_TYPES_H
#define DROW_PLUGIN_UI_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/utils_types.h"
#include "gd/app/app_types.h"
#include "render/runtime/ui_runtime_types.h"
#include "plugin/package/plugin_package_types.h"
#include "plugin/basicanim/plugin_basicanim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_ui_module * plugin_ui_module_t;
typedef struct plugin_ui_env * plugin_ui_env_t;
typedef struct plugin_ui_env_backend * plugin_ui_env_backend_t;
typedef struct plugin_ui_env_action * plugin_ui_env_action_t;
typedef struct plugin_ui_env_action_it * plugin_ui_env_action_it_t;
typedef struct plugin_ui_mouse * plugin_ui_mouse_t;
typedef struct plugin_ui_touch_track * plugin_ui_touch_track_t;
typedef struct plugin_ui_touch_track_it * plugin_ui_touch_track_it_t;
typedef struct plugin_ui_package_queue_managed * plugin_ui_package_queue_managed_t;
typedef struct plugin_ui_package_queue_using * plugin_ui_package_queue_using_t;
typedef struct plugin_ui_page * plugin_ui_page_t;
typedef struct plugin_ui_page_it * plugin_ui_page_it_t;
typedef struct plugin_ui_page_meta * plugin_ui_page_meta_t;
typedef struct plugin_ui_page_eh * plugin_ui_page_eh_t;
typedef struct plugin_ui_page_eh_it * plugin_ui_page_eh_it_t;
typedef struct plugin_ui_page_plugin * plugin_ui_page_plugin_t;
typedef struct plugin_ui_page_plugin_it * plugin_ui_page_plugin_it_t;
typedef struct plugin_ui_page_slot * plugin_ui_page_slot_t;
typedef struct plugin_ui_page_slot_it * plugin_ui_page_slot_it_t;
typedef struct plugin_ui_aspect * plugin_ui_aspect_t;
typedef struct plugin_ui_control * plugin_ui_control_t;
typedef struct plugin_ui_control_binding * plugin_ui_control_binding_t;
typedef struct plugin_ui_control_category * plugin_ui_control_category_t;
typedef struct plugin_ui_control_timer * plugin_ui_control_timer_t;    
typedef struct plugin_ui_control_it * plugin_ui_control_it_t;
typedef struct plugin_ui_control_action * plugin_ui_control_action_t;
typedef struct plugin_ui_control_action_it * plugin_ui_control_action_it_t;
typedef struct plugin_ui_control_meta * plugin_ui_control_meta_t;
typedef struct plugin_ui_control_attr_meta * plugin_ui_control_attr_meta_t;    
typedef struct plugin_ui_control_rctx * plugin_ui_control_rctx_t;
typedef struct plugin_ui_control_frame * plugin_ui_control_frame_t;
typedef struct plugin_ui_control_frame_it * plugin_ui_control_frame_it_t;
typedef struct plugin_ui_phase * plugin_ui_phase_t;
typedef struct plugin_ui_phase_use_page * plugin_ui_phase_use_page_t;
typedef struct plugin_ui_phase_use_popup_def * plugin_ui_phase_use_popup_def_t;
typedef struct plugin_ui_phase_node * plugin_ui_phase_node_t;
typedef struct plugin_ui_phase_node_it * plugin_ui_phase_node_it_t;
typedef struct plugin_ui_state * plugin_ui_state_t;
typedef struct plugin_ui_state_it * plugin_ui_state_it_t;
typedef struct plugin_ui_state_node * plugin_ui_state_node_t;
typedef struct plugin_ui_state_node_it * plugin_ui_state_node_it_t;
typedef struct plugin_ui_navigation * plugin_ui_navigation_t;
typedef struct plugin_ui_navigation_it * plugin_ui_navigation_it_t;
typedef struct plugin_ui_popup * plugin_ui_popup_t;
typedef struct plugin_ui_popup_it * plugin_ui_popup_it_t;
typedef struct plugin_ui_popup_action * plugin_ui_popup_action_t;
typedef struct plugin_ui_popup_action_it * plugin_ui_popup_action_it_t;
typedef struct plugin_ui_popup_def * plugin_ui_popup_def_t;
typedef struct plugin_ui_popup_def_it * plugin_ui_popup_def_it_t;
typedef struct plugin_ui_popup_def_binding * plugin_ui_popup_def_binding_t;
typedef struct plugin_ui_popup_def_binding_it * plugin_ui_popup_def_binding_it_t;
typedef struct plugin_ui_popup_def_binding_attr * plugin_ui_popup_def_binding_attr_t;
typedef struct plugin_ui_popup_def_binding_attr_it * plugin_ui_popup_def_binding_attr_it_t;
typedef struct plugin_ui_animation * plugin_ui_animation_t;
typedef struct plugin_ui_animation_it * plugin_ui_animation_it_t;
typedef struct plugin_ui_animation_control * plugin_ui_animation_control_t;
typedef struct plugin_ui_animation_meta * plugin_ui_animation_meta_t;

typedef struct plugin_ui_anim_control_alpha_in * plugin_ui_anim_control_alpha_in_t;
typedef struct plugin_ui_anim_control_alpha_out * plugin_ui_anim_control_alpha_out_t;
typedef struct plugin_ui_anim_control_scale_in * plugin_ui_anim_control_scale_in_t;
typedef struct plugin_ui_anim_control_scale_out * plugin_ui_anim_control_scale_out_t;
typedef struct plugin_ui_anim_control_scale_to * plugin_ui_anim_control_scale_to_t;
typedef struct plugin_ui_anim_control_move_in * plugin_ui_anim_control_move_in_t;
typedef struct plugin_ui_anim_control_move_out * plugin_ui_anim_control_move_out_t;
    
typedef struct plugin_ui_anim_control_anim * plugin_ui_anim_control_anim_t;
typedef struct plugin_ui_anim_control_move * plugin_ui_anim_control_move_t;
typedef struct plugin_ui_anim_control_scale * plugin_ui_anim_control_scale_t;
typedef struct plugin_ui_anim_control_scroll * plugin_ui_anim_control_scroll_t;

typedef struct plugin_ui_anim_control_frame_scale * plugin_ui_anim_control_frame_scale_t;
typedef struct plugin_ui_anim_control_frame_move * plugin_ui_anim_control_frame_move_t;
    
typedef struct plugin_ui_anim_label_time_duration * plugin_ui_anim_label_time_duration_t;    
typedef struct plugin_ui_anim_progress_bind_control * plugin_ui_anim_progress_bind_control_t;    
typedef struct plugin_ui_anim_frame_alpha * plugin_ui_anim_frame_alpha_t;
typedef struct plugin_ui_anim_frame_move * plugin_ui_anim_frame_move_t;
typedef struct plugin_ui_anim_frame_bind * plugin_ui_anim_frame_bind_t;
    
typedef struct plugin_ui_move_algorithm * plugin_ui_move_algorithm_t;
typedef struct plugin_ui_move_algorithm_meta * plugin_ui_move_algorithm_meta_t;
typedef struct plugin_ui_template_render * plugin_ui_template_render_t;
typedef struct plugin_ui_exec_plan * plugin_ui_exec_plan_t;
typedef struct plugin_ui_exec_action * plugin_ui_exec_action_t;
typedef struct plugin_ui_exec_step * plugin_ui_exec_step_t;
typedef struct plugin_ui_exec_step_to_state * plugin_ui_exec_step_to_state_t;
typedef struct plugin_ui_exec_step_click * plugin_ui_exec_step_click_t;
typedef struct plugin_ui_exec_step_move * plugin_ui_exec_step_move_t;

typedef struct plugin_ui_cfg_button * plugin_ui_cfg_button_t;
    
typedef enum plugin_ui_control_frame_layer {
    plugin_ui_control_frame_layer_back, /*背景 */
    plugin_ui_control_frame_layer_text, /*文字部分 */
    plugin_ui_control_frame_layer_tail,
    plugin_ui_control_frame_layer_float,
} plugin_ui_control_frame_layer_t;

typedef enum plugin_ui_control_frame_usage {
    plugin_ui_control_frame_usage_normal,
    plugin_ui_control_frame_usage_down,
    plugin_ui_control_frame_usage_gray,
} plugin_ui_control_frame_usage_t;
    
typedef enum plugin_ui_navigation_state_op {
    plugin_ui_navigation_state_op_switch = 1,
    plugin_ui_navigation_state_op_call = 2,
    plugin_ui_navigation_state_op_back = 3,
    plugin_ui_navigation_state_op_template = 4,
} plugin_ui_navigation_state_op_t;

typedef enum plugin_ui_navigation_phase_op {
    plugin_ui_navigation_phase_op_switch = 1,
    plugin_ui_navigation_phase_op_call = 2,
    plugin_ui_navigation_phase_op_back = 3,
    plugin_ui_navigation_phase_op_reset = 4,
} plugin_ui_navigation_phase_op_t;

typedef enum plugin_ui_navigation_category {
    plugin_ui_navigation_category_state = 1,
    plugin_ui_navigation_category_phase = 2,
} plugin_ui_navigation_category_t;

typedef enum plugin_ui_navigation_state_base_policy {
	plugin_ui_navigation_state_base_curent = 0,
	plugin_ui_navigation_state_base_top = 1,
} plugin_ui_navigation_state_base_policy_t;

typedef enum plugin_ui_renter_policy {
    plugin_ui_renter_skip = 1,
    plugin_ui_renter_go = 2,
    plugin_ui_renter_back = 3,
} plugin_ui_renter_policy_t;
    
typedef enum plugin_ui_page_eh_scope {
    plugin_ui_page_eh_scope_visible,
    plugin_ui_page_eh_scope_all,
} plugin_ui_page_eh_scope_t;
    
typedef int (*plugin_ui_page_init_fun_t)(plugin_ui_page_t page);
typedef void (*plugin_ui_page_fini_fun_t)(plugin_ui_page_t page);

typedef enum plugin_ui_page_load_policy {
    plugin_ui_page_load_policy_phase,
    plugin_ui_page_load_policy_visiable,
} plugin_ui_page_load_policy_t;
    
typedef enum plugin_ui_screen_resize_policy {
    plugin_ui_screen_resize_by_width,
    plugin_ui_screen_resize_by_height,
    plugin_ui_screen_resize_free,
} plugin_ui_screen_resize_policy_t;
    
typedef enum plugin_ui_event {
    plugin_ui_event_all = 0,
    plugin_ui_event_min = 1,
    plugin_ui_event_mouse_down = 1,
    plugin_ui_event_mouse_up,
    plugin_ui_event_mouse_move,
    plugin_ui_event_mouse_click,
    plugin_ui_event_mouse_double_click,
    plugin_ui_event_mouse_long_push,    
    plugin_ui_event_move_begin,
    plugin_ui_event_move_moving,
    plugin_ui_event_move_done,
    plugin_ui_event_gain_focus,
    plugin_ui_event_lost_focus,
    plugin_ui_event_float_begin,  /*移入 */
    plugin_ui_event_float_done, /*移出 */
    plugin_ui_event_float_move, /*移动 */
    plugin_ui_event_show,
    plugin_ui_event_hide,
    plugin_ui_event_list_item_show,
    plugin_ui_event_list_head_show,
    plugin_ui_event_list_tail_show,
    plugin_ui_event_list_selection_changed,    
    plugin_ui_event_toggle_click,
    plugin_ui_event_switch_changed,
    plugin_ui_event_slider_changed,
    plugin_ui_event_swiper_changed,
    plugin_ui_event_radiobox_changed,
    plugin_ui_event_checkbox_changed,
    plugin_ui_event_editorbox_changed,
    plugin_ui_event_editorbox_enter,
    plugin_ui_event_vscroll_changed,
    plugin_ui_event_hscroll_changed,
    plugin_ui_event_progress_changed,
    plugin_ui_event_progress_done,
    plugin_ui_event_max,
} plugin_ui_event_t;

typedef enum plugin_ui_event_scope {
    plugin_ui_event_scope_self = 1,
    plugin_ui_event_scope_childs = 2,
    plugin_ui_event_scope_all = 3,
} plugin_ui_event_scope_t;

typedef enum plugin_ui_animation_state {
    plugin_ui_animation_state_init,
    plugin_ui_animation_state_waiting,
    plugin_ui_animation_state_working,
    plugin_ui_animation_state_done,
    plugin_ui_animation_state_keep,
} plugin_ui_animation_state_t;

typedef enum plugin_ui_control_move_pos {
    plugin_ui_control_move_left = 1,
    plugin_ui_control_move_right,
    plugin_ui_control_move_top,
    plugin_ui_control_move_bottom,
    plugin_ui_control_move_left_top,
    plugin_ui_control_move_left_bottom,
    plugin_ui_control_move_right_top,
    plugin_ui_control_move_right_bottom,
} plugin_ui_control_move_pos_t;

typedef enum plugin_ui_package_scope {
    plugin_ui_package_cur_phase = 1,
    plugin_ui_package_next_phase = 2,
    plugin_ui_package_next_phase_loading = 3,
    plugin_ui_package_cur_state = 4,
    plugin_ui_package_next_state = 5,
} plugin_ui_package_scope_t;

typedef void (*plugin_ui_event_fun_t)(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
typedef void (*plugin_ui_timer_fun_t)(void * ctx, plugin_ui_control_t control, uint16_t timer_type);
typedef void (*plugin_ui_page_eh_fun_t)(void * ctx, plugin_ui_page_t page, LPDRMETA data_meta, void const * data, size_t data_size);
typedef void (*plugin_ui_popup_action_fun_t)(void * ctx, plugin_ui_popup_t popup, const char * action_name);
typedef void (*plugin_ui_animation_fun_t)(void * ctx, plugin_ui_animation_t animation, void * args);

#define PLUGIN_UI_CONTROL_ACTION_DATA_CAPACITY (64)

#ifdef __cplusplus
}
#endif

#endif
