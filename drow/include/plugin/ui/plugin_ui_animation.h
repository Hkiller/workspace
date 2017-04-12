#ifndef DROW_PLUGIN_UI_ANIMATION_H
#define DROW_PLUGIN_UI_ANIMATION_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_animation_it {
    plugin_ui_animation_t (*next)(struct plugin_ui_animation_it * it);
    char m_data[64];
};

plugin_ui_animation_t plugin_ui_animation_create(plugin_ui_env_t env, plugin_ui_animation_meta_t meta);
plugin_ui_animation_t plugin_ui_animation_create_by_type_name(plugin_ui_env_t env, const char * type_name);
void plugin_ui_animation_free(plugin_ui_animation_t animation);

uint32_t plugin_ui_animation_id(plugin_ui_animation_t animation);
plugin_ui_animation_t plugin_ui_animation_find(plugin_ui_env_t env, uint32_t animation_id);

const char * plugin_ui_animation_type_name(plugin_ui_animation_t animation);
    
const char * plugin_ui_animation_name(plugin_ui_animation_t animation);
int plugin_ui_animation_set_name(plugin_ui_animation_t animation, const char * name);
    
plugin_ui_env_t plugin_ui_animation_env(plugin_ui_animation_t animation);
uint8_t plugin_ui_animation_have_visiable_control(plugin_ui_animation_t animation);

uint8_t plugin_ui_animation_auto_free(plugin_ui_animation_t animation);
void plugin_ui_animation_set_auto_free(plugin_ui_animation_t animation, uint8_t auto_free);

float plugin_ui_animation_delay(plugin_ui_animation_t animation);
int plugin_ui_animation_set_delay(plugin_ui_animation_t animation, float delay);
int plugin_ui_animation_set_delay_frame(plugin_ui_animation_t animation, uint32_t frame);

float plugin_ui_animation_loop_delay(plugin_ui_animation_t animation);
uint32_t plugin_ui_animation_loop_count(plugin_ui_animation_t animation);
int plugin_ui_animation_set_loop(plugin_ui_animation_t animation, uint32_t loop_count, float loop_delay);
    
plugin_ui_animation_state_t plugin_ui_animation_state(plugin_ui_animation_t animation);
const char * plugin_ui_animation_state_str(plugin_ui_animation_t animation);    
int plugin_ui_animation_start(plugin_ui_animation_t animation);
int plugin_ui_animation_stop(plugin_ui_animation_t animation);    

int plugin_ui_animation_bind_control(plugin_ui_animation_t animation, plugin_ui_control_t control, uint8_t is_tie);
void * plugin_ui_animation_data(plugin_ui_animation_t animation);
plugin_ui_animation_t plugin_ui_animation_from_data(void * data);

plugin_ui_control_t plugin_ui_animation_find_first_tie_control(plugin_ui_animation_t animation);
void plugin_ui_animation_controls(plugin_ui_animation_t animation, plugin_ui_control_it_t control_it);
    
const char * plugin_ui_animation_state_to_str(plugin_ui_animation_state_t state);

plugin_ui_aspect_t plugin_ui_animation_aspect(plugin_ui_animation_t animation);
plugin_ui_aspect_t plugin_ui_animation_aspect_check_create(plugin_ui_animation_t animation);    
    
int plugin_ui_animation_setup(
    plugin_ui_animation_t animation, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);

int plugin_ui_animation_set_on_complete(
    plugin_ui_animation_t animation, void * ctx, plugin_ui_animation_fun_t fun, void * arg, void (*arg_free)(void *));

/*animation_it*/    
#define plugin_ui_animation_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

