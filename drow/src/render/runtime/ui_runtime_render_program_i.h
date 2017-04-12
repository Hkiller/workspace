#ifndef UI_RUNTIME_SHADER_PROGRAM_I_H
#define UI_RUNTIME_SHADER_PROGRAM_I_H
#include "render/runtime/ui_runtime_render_program.h"
#include "ui_runtime_render_i.h"
#include "ui_runtime_render_program_attr_i.h"

struct ui_runtime_render_program {
    ui_runtime_render_t m_render;
    TAILQ_ENTRY(ui_runtime_render_program) m_next;
    char m_name[64];
    uint32_t m_state_count;
    uint32_t m_attr_flag;
    ui_runtime_render_program_attr_list_t m_attrs;
    ui_runtime_render_program_unif_list_t m_unifs;
    ui_runtime_render_program_unif_t m_unif_buildins[ui_runtime_render_program_unif_buildin_count];
};

void ui_runtime_render_program_free_i(ui_runtime_render_program_t  program, uint8_t is_external_unloaded);
    
#endif
