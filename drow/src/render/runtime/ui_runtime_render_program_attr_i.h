#ifndef UI_RUNTIME_SHADER_PROGRAM_ATTR_I_H
#define UI_RUNTIME_SHADER_PROGRAM_ATTR_I_H
#include "render/runtime/ui_runtime_render_program_attr.h"
#include "ui_runtime_render_program_i.h"

struct ui_runtime_render_program_attr {
    ui_runtime_render_program_t m_program;
    TAILQ_ENTRY(ui_runtime_render_program_attr) m_next;
    ui_runtime_render_program_attr_id_t m_attr_id;
};

#endif
