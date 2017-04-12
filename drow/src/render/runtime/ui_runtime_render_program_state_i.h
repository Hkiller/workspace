#ifndef UI_RUNTIME_SHADER_PROGRAM_STATE_I_H
#define UI_RUNTIME_SHADER_PROGRAM_STATE_I_H
#include "render/runtime/ui_runtime_render_program_state.h"
#include "ui_runtime_render_i.h"

struct ui_runtime_render_program_state {
    ui_runtime_render_t m_render;
    union {
        TAILQ_ENTRY(ui_runtime_render_program_state) m_next;
        struct {
            ui_runtime_render_program_t m_program;
            uint8_t m_is_sorted;
            ui_runtime_render_program_state_attr_list_t m_attrs;
            ui_runtime_render_program_state_unif_list_t m_unifs;
        };
    };
};

void ui_runtime_render_program_state_real_free(ui_runtime_render_program_state_t program_state);

#endif
