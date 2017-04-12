#ifndef UI_RUNTIME_RENDER_PROGRAM_ATTR_H
#define UI_RUNTIME_RENDER_PROGRAM_ATTR_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_program_attr_it {
    ui_runtime_render_program_attr_t (*next)(struct ui_runtime_render_program_attr_it * it);
    char m_data[64];
};

ui_runtime_render_program_attr_t
ui_runtime_render_program_attr_create(ui_runtime_render_program_t program, ui_runtime_render_program_attr_id_t attr_id);
void ui_runtime_render_program_attr_free(ui_runtime_render_program_attr_t attr);
    
ui_runtime_render_program_attr_t
ui_runtime_render_program_attr_find(ui_runtime_render_program_t program, ui_runtime_render_program_attr_id_t attr_id);

ui_runtime_render_program_attr_id_t ui_runtime_render_program_attr_id(ui_runtime_render_program_attr_t attr);
const char * ui_runtime_render_program_attr_id_str(ui_runtime_render_program_attr_t attr);
    
void * ui_runtime_render_program_attr_data(ui_runtime_render_program_attr_t attr);
    
#define ui_runtime_render_program_attr_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
