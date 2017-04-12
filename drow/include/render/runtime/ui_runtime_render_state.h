#ifndef UI_RUNTIME_RENDER_STATE_H
#define UI_RUNTIME_RENDER_STATE_H
#include "render/utils/ui_rect.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_runtime_render_state_tag {
    ui_runtime_render_state_tag_blend,
    ui_runtime_render_state_tag_scissor,
    ui_runtime_render_state_tag_cull_face,
    ui_runtime_render_state_tag_front_face,
    ui_runtime_render_state_tag_depth_test,
    ui_runtime_render_state_tag_depth_write,
    ui_runtime_render_state_tag_depth_func,
    ui_runtime_render_state_tag_stencil_test,
    ui_runtime_render_state_tag_stencil_write,
    ui_runtime_render_state_tag_stencil_func,
    ui_runtime_render_state_tag_stencil_op,
} ui_runtime_render_state_tag_t;

struct ui_runtime_render_state_data {
    uint8_t m_depth_test_enable;
    uint8_t m_depth_write_enable;
    ui_runtime_render_depth_function_t m_depth_function;
    
    uint8_t m_blend_on;
    struct ui_runtime_render_blend m_blend;
    uint8_t m_scissor_on;
    struct ui_rect m_scissor;
    
    ui_runtime_render_cull_face_t m_cull_face;
    ui_runtime_render_front_face_t m_front_face;
    uint8_t m_stencil_test_enabled;
    uint32_t m_stencil_write;
    ui_runtime_render_stencil_function_t mstencil_function;
    //int _stencilFunctionRef;
    uint32_t m_stencil_function_mask;
    ui_runtime_render_stencil_op_t m_stencil_op_S_fail;
    ui_runtime_render_stencil_op_t m_stencil_op_Dp_fail;
    ui_runtime_render_stencil_op_t m_stencil_op_Dp_pass;
    uint32_t m_bits;
};
        
ui_runtime_render_state_t ui_runtime_render_state_parent(ui_runtime_render_state_t state);
ui_runtime_render_state_data_t ui_runtime_render_state_data(ui_runtime_render_state_t state);

ui_runtime_render_state_data_t ui_runtime_render_state_data_find_by_tag(ui_runtime_render_state_t state, ui_runtime_render_state_tag_t tag);
void ui_runtime_render_state_clear_tag(ui_runtime_render_state_t state, ui_runtime_render_state_tag_t tag);

void ui_runtime_render_state_set_scissor(ui_runtime_render_state_t state, ui_rect_t rect);
    
void ui_runtime_render_state_set_blend(ui_runtime_render_state_t state, ui_runtime_render_blend_t blend);
uint8_t ui_runtime_render_state_blend_compatible(ui_runtime_render_state_t state, ui_runtime_render_blend_t blend);
    
uint8_t ui_runtime_render_state_compatible(ui_runtime_render_state_t l, ui_runtime_render_state_t r, uint32_t flags);
    
#ifdef __cplusplus
}
#endif

#endif
