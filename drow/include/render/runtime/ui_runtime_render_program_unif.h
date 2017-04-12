#ifndef UI_RUNTIME_RENDER_PROGRAM_UNIF_H
#define UI_RUNTIME_RENDER_PROGRAM_UNIF_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_program_unif_it {
    ui_runtime_render_program_unif_t (*next)(struct ui_runtime_render_program_unif_it * it);
    char m_data[64];
};

ui_runtime_render_program_unif_t
ui_runtime_render_program_unif_create(
    ui_runtime_render_program_t program, const char * name, ui_runtime_render_program_unif_type_t unif_type);
void ui_runtime_render_program_unif_free(ui_runtime_render_program_unif_t unif);

void * ui_runtime_render_program_unif_data(ui_runtime_render_program_unif_t unif);

const char * ui_runtime_render_program_unif_name(ui_runtime_render_program_unif_t unif);
ui_runtime_render_program_unif_type_t ui_runtime_render_program_unif_type(ui_runtime_render_program_unif_t unif);
ui_runtime_render_program_unif_t ui_runtime_render_program_unif_find(ui_runtime_render_program_t program, const char * name);

#define ui_runtime_render_program_unif_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
