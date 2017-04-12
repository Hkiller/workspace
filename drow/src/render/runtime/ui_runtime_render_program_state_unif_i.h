#ifndef UI_RUNTIME_SHADER_PROGRAM_STATE_UNIF_I_H
#define UI_RUNTIME_SHADER_PROGRAM_STATE_UNIF_I_H
#include "render/runtime/ui_runtime_render_program_state_unif.h"
#include "ui_runtime_render_program_state_i.h"

struct ui_runtime_render_program_state_unif {
    ui_runtime_render_program_state_t m_program_state;
    TAILQ_ENTRY(ui_runtime_render_program_state_unif) m_next;
    ui_runtime_render_program_unif_t m_unif;
    struct ui_runtime_render_program_state_unif_data m_data;
};

void ui_runtime_render_program_state_unif_real_free(ui_runtime_render_program_state_unif_t program_state_unif);

#endif
