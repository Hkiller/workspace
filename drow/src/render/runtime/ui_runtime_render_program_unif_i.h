#ifndef UI_RUNTIME_SHADER_PROGRAM_UNIF_I_H
#define UI_RUNTIME_SHADER_PROGRAM_UNIF_I_H
#include "render/runtime/ui_runtime_render_program_unif.h"
#include "ui_runtime_render_program_i.h"

struct ui_runtime_render_program_unif {
    ui_runtime_render_program_t m_program;
    TAILQ_ENTRY(ui_runtime_render_program_unif) m_next;
    char m_name[64];
    ui_runtime_render_program_unif_type_t m_type;
};

#endif
