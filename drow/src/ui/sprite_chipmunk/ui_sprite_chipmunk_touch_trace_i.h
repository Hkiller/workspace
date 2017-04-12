#ifndef UI_SPRITE_CHIPMUNK_TOUCH_TRACE_I_H
#define UI_SPRITE_CHIPMUNK_TOUCH_TRACE_I_H
#include "ui_sprite_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_touch_trace {
    int32_t m_id;
    uint8_t m_is_ending;
    ui_vector_2 m_last_pt;
    TAILQ_ENTRY(ui_sprite_chipmunk_touch_trace) m_next_for_env;
    ui_sprite_chipmunk_touch_binding_list_t m_bindings;
};

ui_sprite_chipmunk_touch_trace_t ui_sprite_chipmunk_touch_trace_create(ui_sprite_chipmunk_env_t env, int32_t id, ui_vector_2_t last_pt);
ui_sprite_chipmunk_touch_trace_t ui_sprite_chipmunk_touch_trace_find(ui_sprite_chipmunk_env_t env, int32_t id);
    
void ui_sprite_chipmunk_touch_trace_free(ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_touch_trace_t trace);
void ui_sprite_chipmunk_touch_trace_free_all(ui_sprite_chipmunk_env_t env);

#ifdef __cplusplus
}
#endif

#endif
