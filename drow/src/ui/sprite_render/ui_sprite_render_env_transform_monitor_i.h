#ifndef UI_SPRITE_RENDER_ENV_TRANSFORM_MONITOR_I_H
#define UI_SPRITE_RENDER_ENV_TRANSFORM_MONITOR_I_H
#include "ui_sprite_render_env_i.h"

struct ui_sprite_render_env_transform_monitor {
    ui_sprite_render_env_t m_env;
    TAILQ_ENTRY(ui_sprite_render_env_transform_monitor) m_next;
    ui_sprite_render_env_transform_monitor_fun_t m_process_fun;
    void * m_process_ctx;
};

ui_sprite_render_env_transform_monitor_t
ui_sprite_render_env_transform_monitor_create(
    ui_sprite_render_env_t env, ui_sprite_render_env_transform_monitor_fun_t process_fun, void * process_ctx);

void ui_sprite_render_env_transform_monitor_free(ui_sprite_render_env_transform_monitor_t processor);

#endif
