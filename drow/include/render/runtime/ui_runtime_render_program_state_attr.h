#ifndef UI_RUNTIME_RENDER_PROGRAM_STATE_ATTR_H
#define UI_RUNTIME_RENDER_PROGRAM_STATE_ATTR_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_program_state_attr_it {
    ui_runtime_render_program_state_attr_t (*next)(struct ui_runtime_render_program_state_attr_it * it);
    char m_data[64];
};

struct ui_runtime_render_program_state_attr_data {
    uint8_t m_stride;
    uint8_t m_start_pos;
    uint8_t m_element_count;
    uint8_t m_element_type;
};
    
ui_runtime_render_program_state_attr_t
ui_runtime_render_program_state_attr_create(
    ui_runtime_render_program_state_t state,
    ui_runtime_render_program_attr_t attr,
    uint8_t stride,
    uint8_t start_pos,
    uint8_t element_count,
    uint8_t element_type);
    
int ui_runtime_render_program_state_attr_create_by_id_if_exist(
    ui_runtime_render_program_state_t state,
    ui_runtime_render_program_attr_id_t attr_id,
    uint8_t stride,
    uint8_t start_pos,
    uint8_t element_count,
    uint8_t element_type);

void ui_runtime_render_program_state_attr_free(ui_runtime_render_program_state_attr_t program_state_attr);

ui_runtime_render_program_attr_t ui_runtime_render_program_state_attr_attr(ui_runtime_render_program_state_attr_t attr);
ui_runtime_render_program_state_attr_data_t ui_runtime_render_program_state_attr_data(ui_runtime_render_program_state_attr_t attr);

int ui_runtime_render_program_state_attr_data_cmp(ui_runtime_render_program_state_attr_data_t l, ui_runtime_render_program_state_attr_data_t r);
    
#define ui_runtime_render_program_state_attr_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
