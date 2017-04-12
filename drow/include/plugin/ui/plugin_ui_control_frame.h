#ifndef DROW_PLUGIN_UI_CONTROL_FRAME_H
#define DROW_PLUGIN_UI_CONTROL_FRAME_H
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_layout.h"
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_frame_it {
    plugin_ui_control_frame_t (*next)(struct plugin_ui_control_frame_it * it);
    char m_data[64];
};
    
plugin_ui_control_frame_t
plugin_ui_control_frame_create(
    plugin_ui_control_t control,
    plugin_ui_control_frame_layer_t layer,
    plugin_ui_control_frame_usage_t usage,
    ui_runtime_render_obj_ref_t render_obj_ref);

plugin_ui_control_frame_t
plugin_ui_control_frame_create_by_res(
    plugin_ui_control_t control,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, const char * res);

plugin_ui_control_frame_t
plugin_ui_control_frame_create_by_type(
    plugin_ui_control_t control, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, const char * type, const char * args);

plugin_ui_control_frame_t
plugin_ui_control_frame_create_by_def(
    plugin_ui_control_t control,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage,
    UI_CONTROL_RES_REF const * res_ref);

plugin_ui_control_frame_t
plugin_ui_control_frame_create_by_frame(
    plugin_ui_control_t control,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage,
    plugin_ui_control_frame_t from_frame);
    
void plugin_ui_control_frame_free(plugin_ui_control_frame_t frame);

int plugin_ui_control_frame_setup(plugin_ui_control_frame_t frame, char * arg_buf_will_change);
    
void plugin_ui_control_frame_clear(plugin_ui_control_t control, plugin_ui_aspect_t aspect);
void plugin_ui_control_frame_clear_in_layer(plugin_ui_control_t control, plugin_ui_control_frame_layer_t layer, plugin_ui_aspect_t aspect);
void plugin_ui_control_frame_clear_by_usage(plugin_ui_control_t control, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect);
void plugin_ui_control_frame_clear_by_layer_and_usage(
    plugin_ui_control_t control,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect);

uint32_t plugin_ui_control_frame_count_by_usage(plugin_ui_control_t control, plugin_ui_control_frame_usage_t usage);
    
const char * plugin_ui_control_frame_name(plugin_ui_control_frame_t frame);
int plugin_ui_control_frame_set_name(plugin_ui_control_frame_t frame, const char * name);

const char * plugin_ui_control_frame_obj_type_name(plugin_ui_control_frame_t frame);
    
plugin_ui_control_frame_t
plugin_ui_control_frame_find_by_name(plugin_ui_control_t control, const char * name);

plugin_ui_control_frame_t
plugin_ui_control_frame_find_by_render_obj_type(plugin_ui_control_t control, const char * name);

plugin_ui_control_frame_t
plugin_ui_control_frame_find_by_local_pt(plugin_ui_control_t control, ui_vector_2_t pt);

plugin_ui_control_t plugin_ui_control_frame_control(plugin_ui_control_frame_t frame);

plugin_ui_control_frame_layer_t plugin_ui_control_frame_layer(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_layer(plugin_ui_control_frame_t frame, plugin_ui_control_frame_layer_t layer);
const char * plugin_ui_control_frame_layer_str(plugin_ui_control_frame_t frame);
const char * plugin_ui_control_frame_layer_to_str(plugin_ui_control_frame_layer_t layer);
int plugin_ui_control_frame_str_to_layer(const char * str_layer, plugin_ui_control_frame_layer_t * r);

plugin_ui_control_frame_usage_t plugin_ui_control_frame_usage(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_usage(plugin_ui_control_frame_t frame, plugin_ui_control_frame_usage_t usage);
    
float plugin_ui_control_frame_alpha(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_alpha(plugin_ui_control_frame_t frame, float alpha);
    
uint8_t plugin_ui_control_frame_visible(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_visible(plugin_ui_control_frame_t frame, uint8_t visible);

uint8_t plugin_ui_control_frame_draw_inner(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_draw_inner(plugin_ui_control_frame_t frame, uint8_t draw_inner);
    
ui_runtime_render_obj_t plugin_ui_control_frame_render_obj(plugin_ui_control_frame_t frame);
ui_runtime_render_obj_ref_t plugin_ui_control_frame_render_obj_ref(plugin_ui_control_frame_t frame);
void * plugin_ui_control_frame_render_obj_data(plugin_ui_control_frame_t frame);
    
uint8_t plugin_ui_control_frame_base_pos(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_base_pos(plugin_ui_control_frame_t frame, uint8_t pos);

uint8_t plugin_ui_control_frame_auto_remove(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_auto_remove(plugin_ui_control_frame_t frame, uint8_t auto_remove);

ui_vector_2_t plugin_ui_control_frame_base_size(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_base_size(plugin_ui_control_frame_t frame, ui_vector_2_t base_size);

ui_vector_2_t plugin_ui_control_frame_runtime_size(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_runtime_size(plugin_ui_control_frame_t frame, ui_vector_2_t runtime_size);
    
ui_vector_2_t plugin_ui_control_frame_scale(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_scale(plugin_ui_control_frame_t frame, ui_vector_2_t scale);
    
float plugin_ui_control_frame_priority(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_priority(plugin_ui_control_frame_t frame, float priority);

void plugin_ui_control_frames(plugin_ui_control_t control, plugin_ui_control_frame_it_t frame_it);

void plugin_ui_control_frames_in_layer(plugin_ui_control_t control, plugin_ui_control_frame_it_t frame_it, plugin_ui_control_frame_layer_t layer);
void plugin_ui_control_frames_by_usage(plugin_ui_control_t control, plugin_ui_control_frame_it_t frame_it, plugin_ui_control_frame_usage_t usage);
void plugin_ui_control_frames_by_layer_and_usage(
    plugin_ui_control_t control, plugin_ui_control_frame_it_t frame_it,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage);

const char * plugin_ui_control_frame_layer_name(plugin_ui_control_frame_layer_t layer);

void plugin_ui_control_frame_print(write_stream_t s, plugin_ui_control_frame_t frame);
const char * plugin_ui_control_frame_dump(mem_buffer_t buff, plugin_ui_control_frame_t frame);

ui_vector_2_t plugin_ui_control_frame_runtime_size(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_runtime_size(plugin_ui_control_frame_t frame, ui_vector_2_t size);
    
ui_vector_2_t plugin_ui_control_frame_local_pos(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_local_pos(plugin_ui_control_frame_t frame, ui_vector_2_t local_pos);

ui_vector_2 plugin_ui_control_frame_world_pos(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_world_pos(plugin_ui_control_frame_t frame, ui_vector_2_t world_pos);

ui_rect plugin_ui_control_frame_local_rect(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_local_rect(plugin_ui_control_frame_t frame, ui_rect_t rect);

ui_vector_2_t plugin_ui_control_frame_render_size(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_render_size(plugin_ui_control_frame_t frame, ui_vector_2_t size);
    
uint8_t plugin_ui_control_frame_sync_size(plugin_ui_control_frame_t frame);
void plugin_ui_control_frame_set_sync_size(plugin_ui_control_frame_t frame, uint8_t sync_size);

/*animation */
void plugin_ui_control_frame_cancel_animations(plugin_ui_control_frame_t frame, plugin_ui_aspect_t aspect);
plugin_ui_animation_t plugin_ui_control_frame_create_animation(plugin_ui_control_frame_t frame, char * args_will_changed);

/*control_frame_it*/
#define plugin_ui_control_frame_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

