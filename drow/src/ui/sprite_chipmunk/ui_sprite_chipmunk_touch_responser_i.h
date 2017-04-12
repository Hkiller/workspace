#ifndef UI_SPRITE_CHIPMUNK_TOUCH_RESPONSER_I_H
#define UI_SPRITE_CHIPMUNK_TOUCH_RESPONSER_I_H
#include "ui_sprite_chipmunk_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_touch_responser {
    ui_sprite_chipmunk_obj_t m_obj;
    TAILQ_ENTRY(ui_sprite_chipmunk_touch_responser) m_next_for_obj;
    uint8_t m_finger_count;
    uint8_t m_is_capture;
    uint8_t m_is_grab;
    uint8_t m_is_start;
    uint8_t m_is_active;
    uint16_t m_threshold;
    uint8_t m_is_cur_processed;
    uint8_t m_binding_count;
    ui_sprite_chipmunk_touch_binding_list_t m_bindings;

    void (*m_on_begin)(void * responser);
    void (*m_on_move)(void * responser);
    void (*m_on_end)(void * responser);
    void (*m_on_cancel)(void * responser);
};

int ui_sprite_chipmunk_touch_responser_set_finger_count(ui_sprite_chipmunk_touch_responser_t responser, uint8_t finger_count);
int ui_sprite_chipmunk_touch_responser_set_is_capture(ui_sprite_chipmunk_touch_responser_t responser, uint8_t is_capture);
int ui_sprite_chipmunk_touch_responser_set_is_grab(ui_sprite_chipmunk_touch_responser_t responser, uint8_t is_grab);

void ui_sprite_chipmunk_touch_responser_cancel(ui_sprite_chipmunk_touch_responser_t responser);

void ui_sprite_chipmunk_touch_responser_init(ui_sprite_chipmunk_touch_responser_t responser, ui_sprite_entity_t entity, ui_sprite_chipmunk_obj_t obj);
void ui_sprite_chipmunk_touch_responser_fini(ui_sprite_chipmunk_touch_responser_t responser);
int ui_sprite_chipmunk_touch_responser_copy(ui_sprite_chipmunk_touch_responser_t to, ui_sprite_chipmunk_touch_responser_t from);
void ui_sprite_chipmunk_touch_responser_free(ui_sprite_chipmunk_touch_responser_t responser);
    
int ui_sprite_chipmunk_touch_responser_enter(ui_sprite_chipmunk_touch_responser_t responser);
void ui_sprite_chipmunk_touch_responser_exit(ui_sprite_chipmunk_touch_responser_t responser);

void ui_sprite_chipmunk_touch_responser_on_begin(ui_sprite_chipmunk_touch_responser_t responser);
void ui_sprite_chipmunk_touch_responser_on_move(ui_sprite_chipmunk_touch_responser_t responser);
void ui_sprite_chipmunk_touch_responser_on_end(ui_sprite_chipmunk_touch_responser_t responser);

#ifdef __cplusplus
}
#endif

#endif
