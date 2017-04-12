#ifndef UI_SPRITE_RENDER_ENV_TOUCH_PROCESSOR_I_H
#define UI_SPRITE_RENDER_ENV_TOUCH_PROCESSOR_I_H
#include "ui_sprite_render_env_i.h"

struct ui_sprite_render_env_touch_processor {
    ui_sprite_render_env_t m_env;
    TAILQ_ENTRY(ui_sprite_render_env_touch_processor) m_next;
    ui_sprite_render_env_touch_process_fun_t m_process_fun;
    void * m_process_ctx;
};

ui_sprite_render_env_touch_processor_t
ui_sprite_render_env_touch_processor_create(
    ui_sprite_render_env_t env, ui_sprite_render_env_touch_process_fun_t process_fun, void * process_ctx);

void ui_sprite_render_env_touch_processor_free(ui_sprite_render_env_touch_processor_t processor);
 
void ui_sprite_render_env_touch_process(
    void * ctx, ui_runtime_render_obj_t obj,
    uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t screen_pt, ui_vector_2_t logic_pt);

#endif
