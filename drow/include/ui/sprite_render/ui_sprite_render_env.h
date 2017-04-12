#ifndef UI_SPRITE_RENDER_ENV_H
#define UI_SPRITE_RENDER_ENV_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_ENV_NAME;

ui_sprite_render_env_t ui_sprite_render_env_create(ui_sprite_render_module_t module, ui_sprite_world_t world);
void ui_sprite_render_env_free(ui_sprite_world_t world);

ui_sprite_render_env_t ui_sprite_render_env_find(ui_sprite_world_t world);

int ui_sprite_render_env_set_update_priority(ui_sprite_render_env_t env, int8_t priority);
void ui_sprite_render_env_set_debug(ui_sprite_render_env_t env, uint8_t debug);

ui_vector_2_t ui_sprite_render_env_design_size(ui_sprite_render_env_t env);
void ui_sprite_render_env_set_design_size(ui_sprite_render_env_t env, ui_vector_2_t sz);
    
ui_vector_2_t ui_sprite_render_env_size(ui_sprite_render_env_t env);
void ui_sprite_render_env_set_size(ui_sprite_render_env_t env, ui_vector_2_t sz);
    
ui_transform_t ui_sprite_render_env_base_transform(ui_sprite_render_env_t env);
void ui_sprite_render_env_set_base_transform(ui_sprite_render_env_t env, ui_transform_t transform);

ui_transform_t ui_sprite_render_env_transform(ui_sprite_render_env_t env);
void ui_sprite_render_env_set_transform(ui_sprite_render_env_t env, ui_transform_t transform);
int ui_sprite_render_env_add_transform_monitor(ui_sprite_render_env_t env, ui_sprite_render_env_transform_monitor_fun_t process_fun, void * process_ctx);
int ui_sprite_render_env_remove_transform_monitor(ui_sprite_render_env_t env, ui_sprite_render_env_transform_monitor_fun_t process_fun, void * process_ctx);
    
ui_vector_2 ui_sprite_render_env_screen_to_world(ui_sprite_render_env_t env, ui_vector_2_t pos);
ui_vector_2 ui_sprite_render_env_world_to_screen(ui_sprite_render_env_t env, ui_vector_2_t pos);

ui_vector_2 ui_sprite_render_env_logic_to_world(ui_sprite_render_env_t env, ui_vector_2_t pos);
ui_vector_2 ui_sprite_render_env_world_to_logic(ui_sprite_render_env_t env, ui_vector_2_t pos);

ui_vector_2 ui_sprite_render_env_screen_to_logic(ui_sprite_render_env_t env, ui_vector_2_t pos);
ui_vector_2 ui_sprite_render_env_logic_to_screen(ui_sprite_render_env_t env, ui_vector_2_t pos);

int ui_sprite_render_env_add_touch_processor(ui_sprite_render_env_t env, ui_sprite_render_env_touch_process_fun_t process_fun, void * process_ctx);
int ui_sprite_render_env_remove_touch_processor(ui_sprite_render_env_t env, ui_sprite_render_env_touch_process_fun_t process_fun, void * process_ctx);

void ui_sprite_render_env_layers(ui_sprite_render_env_t env, ui_sprite_render_layer_it_t layer_it);

#ifdef __cplusplus
}
#endif

#endif
