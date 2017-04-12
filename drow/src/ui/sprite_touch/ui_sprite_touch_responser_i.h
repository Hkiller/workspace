#ifndef UI_SPRITE_TOUCH_RESPONSER_I_H
#define UI_SPRITE_TOUCH_RESPONSER_I_H
#include "ui_sprite_touch_trace_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_responser_binding {
    ui_sprite_touch_trace_t m_trace;
    ui_sprite_touch_responser_t m_responser;
    TAILQ_ENTRY(ui_sprite_touch_responser_binding) m_next_for_trace;
    TAILQ_ENTRY(ui_sprite_touch_responser_binding) m_next_for_responser;

    ui_vector_2 m_start_screen_pt;
    ui_vector_2 m_start_world_pt;
    ui_vector_2 m_pre_screen_pt;
    ui_vector_2 m_pre_world_pt;
    ui_vector_2 m_cur_screen_pt;
    ui_vector_2 m_cur_world_pt;
};

struct ui_sprite_touch_responser {
    ui_sprite_touch_touchable_t m_touchable;
    float m_z;
    uint8_t m_finger_count;
    uint8_t m_is_capture;
    uint8_t m_is_grab;
    uint8_t m_is_start;
    uint16_t m_threshold;
    TAILQ_ENTRY(ui_sprite_touch_responser) m_next_for_touchable;
    TAILQ_ENTRY(ui_sprite_touch_responser) m_next_for_env;

    uint8_t m_binding_count;
    ui_sprite_touch_responser_binding_list_t m_bindings;

    void (*m_on_begin)(void * responser);
    void (*m_on_move)(void * responser);
    void (*m_on_end)(void * responser);
    void (*m_on_cancel)(void * responser);
};

void ui_sprite_touch_responser_free(ui_sprite_touch_responser_t responser);
int ui_sprite_touch_responser_set_finger_count(ui_sprite_touch_responser_t responser, uint8_t finger_count);
int ui_sprite_touch_responser_set_is_capture(ui_sprite_touch_responser_t responser, uint8_t is_capture);
int ui_sprite_touch_responser_set_is_grab(ui_sprite_touch_responser_t responser, uint8_t is_grab);

void ui_sprite_touch_responser_cancel(ui_sprite_touch_responser_t responser);

int ui_sprite_touch_responser_init(
    ui_sprite_touch_responser_t responser,
    ui_sprite_entity_t entity,
    ui_sprite_touch_touchable_t touchable);

int ui_sprite_touch_responser_copy(ui_sprite_touch_responser_t to, ui_sprite_touch_responser_t from);
void ui_sprite_touch_responser_fini(ui_sprite_touch_responser_t responser);
int ui_sprite_touch_responser_enter(ui_sprite_touch_responser_t responser);
void ui_sprite_touch_responser_exit(ui_sprite_touch_responser_t responser);

ui_sprite_touch_responser_binding_t ui_sprite_touch_responser_bind_tracer(ui_sprite_touch_responser_t responser, ui_sprite_touch_trace_t trace);
void ui_sprite_touch_responser_unbind_tracer(ui_sprite_touch_responser_binding_t binding);
ui_sprite_touch_responser_binding_t ui_sprite_touch_responser_binding_find(ui_sprite_touch_responser_t responser, ui_sprite_touch_trace_t trace);

void ui_sprite_touch_responser_on_begin(ui_sprite_touch_responser_t responser);
void ui_sprite_touch_responser_on_move(ui_sprite_touch_responser_t responser);
void ui_sprite_touch_responser_on_end(ui_sprite_touch_responser_t responser);

#ifdef __cplusplus
}
#endif

#endif
