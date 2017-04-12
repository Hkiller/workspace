#ifndef DROW_PLUGIN_UI_CONTROL_H
#define DROW_PLUGIN_UI_CONTROL_H
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "render/utils/ui_rect.h"
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_it {
    plugin_ui_control_t (*next)(struct plugin_ui_control_it * it);
    char m_data[64];
};
    
plugin_ui_control_t plugin_ui_control_create(plugin_ui_page_t page, uint8_t control_type);
    
void plugin_ui_control_free(plugin_ui_control_t ui_control);

void * plugin_ui_control_product(plugin_ui_control_t ui_control);
uint32_t plugin_ui_control_product_capacity(plugin_ui_control_t ui_control);

plugin_ui_control_t plugin_ui_control_from_product(void * data);

mem_allocrator_t plugin_ui_control_allocrator(plugin_ui_control_t control);
gd_app_context_t plugin_ui_control_app(plugin_ui_control_t control);
plugin_ui_env_t plugin_ui_control_env(plugin_ui_control_t control);    
plugin_ui_page_t plugin_ui_control_page(plugin_ui_control_t control);
ui_runtime_module_t plugin_ui_control_runtime(plugin_ui_control_t control);
ui_cache_manager_t plugin_ui_control_cache_mgr(plugin_ui_control_t control);
ui_data_mgr_t plugin_ui_control_data_mgr(plugin_ui_control_t control);

void plugin_ui_control_set_cache_flag_layout(plugin_ui_control_t control, uint8_t layout);

void plugin_ui_control_adj_by_new_parent(plugin_ui_control_t control, plugin_ui_control_t parent);

/*basic*/
uint8_t plugin_ui_control_type(plugin_ui_control_t control);

const char * plugin_ui_control_name(plugin_ui_control_t control);
void plugin_ui_control_set_name(plugin_ui_control_t control, const char * name);

const char * plugin_ui_control_path_dump(mem_buffer_t buffer, plugin_ui_control_t control);

uint8_t plugin_ui_control_is_name_eq_no_category(plugin_ui_control_t control, const char * name);
uint8_t plugin_ui_control_name_is_eq_no_category(plugin_ui_env_t env, const char * control_name, const char * name);
uint8_t plugin_ui_control_is_attr_eq(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value);
uint8_t plugin_ui_control_is_attr_eq_str(plugin_ui_control_t control, const char * attr_name, const char * attr_value);
    
const char * plugin_ui_control_user_text(plugin_ui_control_t control);
void plugin_ui_control_set_user_text(plugin_ui_control_t control, const char * user_text);

ui_data_control_anim_t plugin_ui_control_find_anim_data(plugin_ui_control_t control, uint8_t anim_type);
    
uint8_t plugin_ui_control_visible(plugin_ui_control_t control);
void plugin_ui_control_set_visible(plugin_ui_control_t control, uint8_t visible);
plugin_ui_control_t plugin_ui_control_hide_by(plugin_ui_control_t control);
    
uint8_t plugin_ui_control_enable(plugin_ui_control_t control);
void plugin_ui_control_set_enable(plugin_ui_control_t control, uint8_t enable);

uint8_t plugin_ui_control_force_clicp(plugin_ui_control_t control);
void plugin_ui_control_set_force_clicp(plugin_ui_control_t control, uint8_t force_clicp);
    
uint8_t plugin_ui_control_accept_click(plugin_ui_control_t control);
void plugin_ui_control_set_accept_click(plugin_ui_control_t control, uint8_t accept_click);

uint8_t plugin_ui_control_accept_float(plugin_ui_control_t control);
void plugin_ui_control_set_accept_float(plugin_ui_control_t control, uint8_t accept_float);

uint8_t plugin_ui_control_accept_global_down_color(plugin_ui_control_t control);
void plugin_ui_control_set_accept_global_down_color(plugin_ui_control_t control, uint8_t accept_global_down_color);
    
uint8_t plugin_ui_control_has_focus(plugin_ui_control_t control);
void plugin_ui_control_set_focus(plugin_ui_control_t control);

uint8_t plugin_ui_control_is_float(plugin_ui_control_t control);
void plugin_ui_control_set_float(plugin_ui_control_t control, uint8_t is_float);
    
uint8_t plugin_ui_control_align_vert(plugin_ui_control_t control);
void plugin_ui_control_set_align_vert(plugin_ui_control_t control, uint8_t align_vert);

uint8_t plugin_ui_control_align_horz(plugin_ui_control_t control);
void plugin_ui_control_set_align_horz(plugin_ui_control_t control, uint8_t align_horz);

ui_vector_2_t plugin_ui_control_editor_pt(plugin_ui_control_t control);
ui_vector_2_t plugin_ui_control_editor_sz(plugin_ui_control_t control);
ui_rect_t plugin_ui_control_editor_pd(plugin_ui_control_t control);

ui_vector_2_t plugin_ui_control_pivot(plugin_ui_control_t control);
void plugin_ui_control_set_pivot(plugin_ui_control_t control, ui_vector_2_t pivot);

ui_vector_3_t plugin_ui_control_angle(plugin_ui_control_t control);
void plugin_ui_control_set_angle(plugin_ui_control_t control, ui_vector_3_t angle);
    
ui_vector_2_t plugin_ui_control_scale(plugin_ui_control_t control);
void plugin_ui_control_set_scale(plugin_ui_control_t control, ui_vector_2_t scale);

ui_vector_2_t plugin_ui_control_all_frame_pos(plugin_ui_control_t control);
void plugin_ui_control_set_all_frame_pos(plugin_ui_control_t control, ui_vector_2_t pos);
    
ui_vector_2_t plugin_ui_control_all_frame_scale(plugin_ui_control_t control);
void plugin_ui_control_set_all_frame_scale(plugin_ui_control_t control, ui_vector_2_t scale);

uint8_t plugin_ui_control_draw_align(plugin_ui_control_t control);
void plugin_ui_control_set_draw_align(plugin_ui_control_t control, uint8_t draw_align);
    
uint8_t plugin_ui_control_draw_frame(plugin_ui_control_t control);
void plugin_ui_control_set_draw_frame(plugin_ui_control_t control, uint8_t draw_frame);

uint8_t plugin_ui_control_draw_inner(plugin_ui_control_t control);
void plugin_ui_control_set_draw_inner(plugin_ui_control_t control, uint8_t draw_inner);
    
uint8_t plugin_ui_control_accept_ptls(plugin_ui_control_t control);
void plugin_ui_control_set_accept_ptls(plugin_ui_control_t control, uint8_t accept_ptls);

uint8_t plugin_ui_control_accept_szls(plugin_ui_control_t control);
void plugin_ui_control_set_accept_szls(plugin_ui_control_t control, uint8_t accept_szls);

uint8_t plugin_ui_control_accept_move(plugin_ui_control_t control);
void plugin_ui_control_set_accept_move(plugin_ui_control_t control, uint8_t accept_move);

uint8_t plugin_ui_control_accept_clip(plugin_ui_control_t control);
void plugin_ui_control_set_accept_clip(plugin_ui_control_t control, uint8_t accept_clip);

uint8_t plugin_ui_control_parent_clip(plugin_ui_control_t control);
void plugin_ui_control_set_parent_clip(plugin_ui_control_t control, uint8_t parent_clip);

ui_color const * plugin_ui_control_effective_color(plugin_ui_control_t control);
    
ui_color const * plugin_ui_control_color(plugin_ui_control_t control);
void plugin_ui_control_set_color(plugin_ui_control_t control, ui_color const * color);

ui_color const * plugin_ui_control_gray_color(plugin_ui_control_t control);
void plugin_ui_control_set_gray_color(plugin_ui_control_t control, ui_color const * gray_color);
    
float plugin_ui_control_alpha(plugin_ui_control_t control);
void plugin_ui_control_set_alpha(plugin_ui_control_t control, float alpha);

uint8_t plugin_ui_control_accept_global_resize(plugin_ui_control_t control);
void plugin_ui_control_set_accept_global_resize(plugin_ui_control_t control, uint8_t accept);

ui_vector_2 plugin_ui_control_pt_world_to_local(plugin_ui_control_t control, ui_vector_2_t pt);
    
/*分发事件 */
typedef enum plugin_ui_event_dispatch_scope {
    plugin_ui_event_dispatch_to_self = 1,
    plugin_ui_event_dispatch_to_parent = 2,
    plugin_ui_event_dispatch_to_self_and_parent = 3,
} plugin_ui_event_dispatch_scope_t;

void plugin_ui_control_dispatch_event(
    plugin_ui_control_t at, plugin_ui_control_t from,
    plugin_ui_event_t evt, plugin_ui_event_dispatch_scope_t scope);
    
/*控件左上角点相关参数 */    
UI_UNIT_VECTOR_2 const * plugin_ui_control_render_pt(plugin_ui_control_t control);
void plugin_ui_control_set_render_pt(plugin_ui_control_t control, UI_UNIT_VECTOR_2 const * pt);
void plugin_ui_control_set_render_pt_x(plugin_ui_control_t control, UI_UNIT const * pt_x);
void plugin_ui_control_set_render_pt_y(plugin_ui_control_t control, UI_UNIT const * pt_y);
void plugin_ui_control_set_render_pt_by_real(plugin_ui_control_t control, ui_vector_2_t real_pt);
void plugin_ui_control_set_render_pt_x_by_real(plugin_ui_control_t control, float x);    
void plugin_ui_control_set_render_pt_y_by_real(plugin_ui_control_t control, float y);
    
ui_vector_2_t plugin_ui_control_real_pt_to_p(plugin_ui_control_t control);
ui_vector_2 plugin_ui_control_real_pt_to_p_no_screen_adj(plugin_ui_control_t control);
ui_vector_2_t plugin_ui_control_real_pt_abs(plugin_ui_control_t control);    
void plugin_ui_control_set_render_pt_abs(plugin_ui_control_t control, ui_vector_2_t real_pt);

/*控件大小数据 */
UI_UNIT_VECTOR_2 const * plugin_ui_control_render_sz(plugin_ui_control_t control);
void plugin_ui_control_set_render_sz(plugin_ui_control_t control, UI_UNIT_VECTOR_2 const * sz);
void plugin_ui_control_set_render_sz_w(plugin_ui_control_t control, UI_UNIT const * sz_w);
void plugin_ui_control_set_render_sz_h(plugin_ui_control_t control, UI_UNIT const * sz_h);
ui_vector_2_t plugin_ui_control_real_sz_no_scale(plugin_ui_control_t control);
ui_vector_2_t plugin_ui_control_real_sz_abs(plugin_ui_control_t control);   
void plugin_ui_control_set_render_sz_by_real(plugin_ui_control_t control, ui_vector_2_t sz);
void plugin_ui_control_set_render_sz_w_by_real(plugin_ui_control_t control, float w);
void plugin_ui_control_set_render_sz_h_by_real(plugin_ui_control_t control, float h);
int plugin_ui_control_set_render_sz_by_frames(plugin_ui_control_t control);
    
ui_vector_2 plugin_ui_control_calc_child_real_sz_no_scale(plugin_ui_control_t control, ui_data_control_t child_data);
ui_vector_2 plugin_ui_control_calc_local_pt_by_policy(plugin_ui_control_t control, uint8_t policy);

/*根据绝对位置设置控件 */
int plugin_ui_control_place_on_screen(
    plugin_ui_control_t control, ui_vector_2_t pivot_screen_pt, ui_rect_t screen_rect);

/*控件位置其他相关函数 */    
ui_rect plugin_ui_control_real_rt_no_scale(plugin_ui_control_t control);
ui_rect plugin_ui_control_real_rt_abs(plugin_ui_control_t control);    
uint8_t plugin_ui_control_contain_test(plugin_ui_control_t control, ui_vector_2_t pt);
ui_rect_t plugin_ui_control_cliped_rt_abs(plugin_ui_control_t control);

/*内部区域 */
ui_rect plugin_ui_control_real_inner_abs(plugin_ui_control_t control);

/*控件逻辑大小相关参数 */
ui_vector_2_t plugin_ui_control_client_real_sz(plugin_ui_control_t control);
void plugin_ui_control_set_client_real_sz(plugin_ui_control_t control, ui_vector_2_t sz);
ui_rect_t plugin_ui_control_client_real_pd(plugin_ui_control_t control);

/*渲染相关参数 */
ui_color_t plugin_ui_control_render_color(plugin_ui_control_t control);
ui_vector_2_t plugin_ui_control_render_scale(plugin_ui_control_t control);
ui_vector_3_t plugin_ui_control_render_angle(plugin_ui_control_t control);

void plugin_ui_control_set_usage_render(plugin_ui_control_t control, plugin_ui_control_frame_usage_t usage, uint8_t is_enable);

uint8_t plugin_ui_control_frame_changed(plugin_ui_control_t control);
    
/*更新控件相关数据 */
uint8_t plugin_ui_control_need_update_cache(plugin_ui_control_t control);
void plugin_ui_control_update_cache(plugin_ui_control_t control, uint16_t flag);
void plugin_ui_control_check_update_from_root(plugin_ui_control_t control);    

/*触控数据 */
plugin_ui_touch_track_t plugin_ui_control_touch_track(plugin_ui_control_t control);
uint8_t plugin_ui_control_has_catch(plugin_ui_control_t control);
uint8_t plugin_ui_control_has_catch_r(plugin_ui_control_t control);
    
/*tree*/
plugin_ui_control_t plugin_ui_control_parent(plugin_ui_control_t control);
uint16_t plugin_ui_control_child_count(plugin_ui_control_t control);
void plugin_ui_control_childs(plugin_ui_control_t control, plugin_ui_control_it_t child_it);
void plugin_ui_control_childs_reverse(plugin_ui_control_t control, plugin_ui_control_it_t child_it);

uint8_t plugin_ui_control_is_child_of_r(plugin_ui_control_t self, plugin_ui_control_t check_parent);

int plugin_ui_control_adj_before(plugin_ui_control_t control, plugin_ui_control_t before_control);
int plugin_ui_control_adj_after(plugin_ui_control_t control, plugin_ui_control_t after_control);    
void plugin_ui_control_adj_top(plugin_ui_control_t control);
void plugin_ui_control_adj_tail(plugin_ui_control_t control);

void plugin_ui_control_add_child_tail(plugin_ui_control_t parent, plugin_ui_control_t child);
void plugin_ui_control_add_child_head(plugin_ui_control_t parent, plugin_ui_control_t child);
void plugin_ui_control_remove_child(plugin_ui_control_t parent, plugin_ui_control_t child);
void plugin_ui_control_remove_childs(plugin_ui_control_t parent);
void plugin_ui_control_destory_childs(plugin_ui_control_t parent);

plugin_ui_control_t plugin_ui_control_find_parent_by_name(plugin_ui_control_t control, const char * name);
plugin_ui_control_t plugin_ui_control_find_parent_by_name_r(plugin_ui_control_t control, const char * name);

plugin_ui_control_t plugin_ui_control_find_parent_by_name_no_category(plugin_ui_control_t control, const char * name);
plugin_ui_control_t plugin_ui_control_find_parent_by_name_no_category_r(plugin_ui_control_t control, const char * name);
    
plugin_ui_control_t plugin_ui_control_find_child_by_name(plugin_ui_control_t control, const char * name);
plugin_ui_control_t plugin_ui_control_find_child_by_name_r(plugin_ui_control_t control, const char * name);

plugin_ui_control_t plugin_ui_control_find_child_by_name_no_category(plugin_ui_control_t control, const char * name);
plugin_ui_control_t plugin_ui_control_find_child_by_name_no_category_r(plugin_ui_control_t control, const char * name);

plugin_ui_control_t plugin_ui_control_find_child_by_attr(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value);
plugin_ui_control_t plugin_ui_control_find_child_by_attr_r(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value);

plugin_ui_control_t plugin_ui_control_find_child_by_attr_str(plugin_ui_control_t control, const char * attr_name, const char * attr_value);
plugin_ui_control_t plugin_ui_control_find_child_by_attr_str_r(plugin_ui_control_t control, const char * attr_name, const char * attr_value);

plugin_ui_control_t plugin_ui_control_find_child_by_condition(plugin_ui_control_t control, const char * name, const char * condition);
plugin_ui_control_t plugin_ui_control_find_child_by_condition_r(plugin_ui_control_t control, const char * name, const char * condition);
    
plugin_ui_control_t plugin_ui_control_find_child_by_path(plugin_ui_control_t control, const char * path);
    
plugin_ui_control_t plugin_ui_control_find_child_by_pt(plugin_ui_control_t control, ui_vector_2_t pt);
plugin_ui_control_t plugin_ui_control_find_child_by_pt_r(plugin_ui_control_t control, ui_vector_2_t pt);

plugin_ui_control_t plugin_ui_control_find_click(plugin_ui_control_t control, ui_vector_2_t pt);
plugin_ui_control_t plugin_ui_control_find_float(plugin_ui_control_t control, ui_vector_2_t pt);

plugin_ui_control_t plugin_ui_control_find_by_path(plugin_ui_env_t env, const char * path);
    
enum plugin_ui_control_visit_tree_policy {
    plugin_ui_control_visit_tree_bfs, /*广度优先 */
    plugin_ui_control_visit_tree_dfs, /*深度优先 */    
};
    
void plugin_ui_control_visit_tree(
    plugin_ui_control_t control,
    enum plugin_ui_control_visit_tree_policy policy,
    void * ctx, int (*fun)(plugin_ui_control_t c, void * ctx),
    uint8_t include_self);

int plugin_ui_control_play_event_sfx(plugin_ui_control_t control, plugin_ui_event_t evt);

const char * plugin_ui_control_msg(plugin_ui_control_t control, uint32_t msg_id);

/*attr*/
int plugin_ui_control_set_attr(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value);
int plugin_ui_control_set_attr_by_str(plugin_ui_control_t control, const char * attr_name, const char * attr_value);
int plugin_ui_control_get_attr(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value);
int plugin_ui_control_bulk_set_attrs(plugin_ui_control_t control, const char * attrs);
    
/*basic layout*/
void plugin_ui_control_basic_layout(plugin_ui_control_t control, ui_vector_2_t client_sz);

/*plugin*/
plugin_ui_aspect_t plugin_ui_control_lock_aspect(plugin_ui_control_t control);
    
/*scroll*/
ui_vector_2_t plugin_ui_control_scroll(plugin_ui_control_t control);
void plugin_ui_control_set_scroll(plugin_ui_control_t control, ui_vector_2_t scroll);
void plugin_ui_control_set_scroll_x(plugin_ui_control_t control, float scroll_x);
void plugin_ui_control_set_scroll_y(plugin_ui_control_t control, float scroll_y);
    
/*tempmlate*/
ui_data_control_t plugin_ui_control_template(plugin_ui_control_t control);
int plugin_ui_control_set_template(plugin_ui_control_t control, ui_data_control_t template_data);
ui_data_control_t plugin_ui_control_find_template(plugin_ui_module_t module, const char * res);

plugin_ui_control_t plugin_ui_control_template_root(plugin_ui_control_t control);
    
/*load*/
ui_data_control_t plugin_ui_control_src(plugin_ui_control_t control);
int plugin_ui_control_load_tree(plugin_ui_control_t ui_control, ui_data_control_t data_control);
int plugin_ui_control_load_childs(plugin_ui_control_t ui_control, ui_data_control_t data_control);
ui_data_control_t plugin_ui_control_data_src(plugin_ui_control_t control); /*template or src*/

/*color*/
plugin_ui_control_frame_t plugin_ui_control_draw_color(plugin_ui_control_t ui_control);
plugin_ui_control_frame_t plugin_ui_control_set_draw_color(plugin_ui_control_t control, uint8_t draw_color);
    
/*animation*/
void plugin_ui_control_cancel_animations(plugin_ui_control_t control, plugin_ui_aspect_t aspect);

plugin_ui_animation_t plugin_ui_control_create_animation(plugin_ui_control_t control, char * args_will_changed);
plugin_ui_animation_t plugin_ui_control_find_animation_by_name(plugin_ui_control_t control, const char * name);
plugin_ui_animation_t plugin_ui_control_find_animation_by_type_name(plugin_ui_control_t control, const char * type_name);

void plugin_ui_control_animations(
    plugin_ui_control_t control, plugin_ui_animation_it_t anim_it, const char * type_name, plugin_ui_aspect_t aspect);

/*navigation */
plugin_ui_navigation_t
plugin_ui_navigation_find_by_control(plugin_ui_state_t state, plugin_ui_control_t control, dr_data_t data);

plugin_ui_navigation_t
plugin_ui_navigation_find_by_path(plugin_ui_state_t state, const char * path, dr_data_t data);
    
int plugin_ui_control_trigger_navigation(plugin_ui_control_t control, dr_data_t data);

/*control_it*/    
#define plugin_ui_control_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

