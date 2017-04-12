#ifndef UI_RUNTIME_SHADER_PROGRAM_STATE_ATTR_I_H
#define UI_RUNTIME_SHADER_PROGRAM_STATE_ATTR_I_H
#include "render/runtime/ui_runtime_render_program_state_attr.h"
#include "ui_runtime_render_program_state_i.h"

struct ui_runtime_render_program_state_attr {
    ui_runtime_render_program_state_t m_program_state;
    TAILQ_ENTRY(ui_runtime_render_program_state_attr) m_next;
    ui_runtime_render_program_attr_t m_attr;
    struct ui_runtime_render_program_state_attr_data m_data;
};

void ui_runtime_render_program_state_attr_real_free(ui_runtime_render_program_state_attr_t program_state_attr);

#endif
