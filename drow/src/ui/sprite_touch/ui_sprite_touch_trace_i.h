#ifndef UI_SPRITE_TOUCH_TRACE_I_H
#define UI_SPRITE_TOUCH_TRACE_I_H
#include "ui/sprite_touch/ui_sprite_touch_trace.h"
#include "ui_sprite_touch_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_trace {
    int32_t m_id;
    uint8_t m_is_ending;
    ui_vector_2 m_last_screen_pt;
    ui_vector_2 m_last_world_pt;
    TAILQ_ENTRY(ui_sprite_touch_trace) m_next_for_env;
    ui_sprite_touch_responser_binding_list_t m_bindings;
};

ui_sprite_touch_trace_t
ui_sprite_touch_trace_create(ui_sprite_touch_env_t env, int32_t id, ui_vector_2_t screen_pt, ui_vector_2_t world_pt);

void ui_sprite_touch_trace_free(ui_sprite_touch_env_t env, ui_sprite_touch_trace_t trace);
void ui_sprite_touch_trace_free_all(ui_sprite_touch_env_t env);

#ifdef __cplusplus
}
#endif

#endif
