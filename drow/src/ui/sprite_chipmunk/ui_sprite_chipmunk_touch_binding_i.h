#ifndef UI_SPRITE_CHIPMUNK_TOUCH_BINDING_I_H
#define UI_SPRITE_CHIPMUNK_TOUCH_BINDING_I_H
#include "ui_sprite_chipmunk_touch_responser_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_touch_binding {
    ui_sprite_chipmunk_touch_trace_t m_trace;
    ui_sprite_chipmunk_touch_responser_t m_responser;
    TAILQ_ENTRY(ui_sprite_chipmunk_touch_binding) m_next_for_trace;
    TAILQ_ENTRY(ui_sprite_chipmunk_touch_binding) m_next_for_responser;
    ui_sprite_chipmunk_obj_body_t m_body;
    ui_vector_2 m_start_world_pt;
    ui_vector_2 m_pre_world_pt;
    ui_vector_2 m_cur_world_pt;
};

ui_sprite_chipmunk_touch_binding_t
ui_sprite_chipmunk_touch_binding_create(ui_sprite_chipmunk_touch_responser_t responser, ui_sprite_chipmunk_touch_trace_t trace, ui_sprite_chipmunk_obj_body_t body);
void ui_sprite_chipmunk_touch_binding_free(ui_sprite_chipmunk_touch_binding_t binding);
ui_sprite_chipmunk_touch_binding_t ui_sprite_chipmunk_touch_binding_find(ui_sprite_chipmunk_touch_responser_t responser, ui_sprite_chipmunk_touch_trace_t trace);

#ifdef __cplusplus
}
#endif

#endif
