#ifndef UI_RUNTIME_RENDER_PROGRAM_H
#define UI_RUNTIME_RENDER_PROGRAM_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_program_t ui_runtime_render_program_create(ui_runtime_render_t render, const char * name);
void ui_runtime_render_program_free(ui_runtime_render_program_t program);

ui_runtime_render_program_t ui_runtime_render_program_find_by_name(ui_runtime_render_t render, const char * name);

void ui_runtime_render_program_clear_attrs_and_unifs(ui_runtime_render_program_t program);

uint32_t ui_runtime_render_program_attr_flag(ui_runtime_render_program_t program);

void ui_runtime_render_program_attrs(ui_runtime_render_program_t program, ui_runtime_render_program_attr_it_t attr_it);
void ui_runtime_render_program_unifs(ui_runtime_render_program_t program, ui_runtime_render_program_unif_it_t unif_it);
    
const char * ui_runtime_render_program_name(ui_runtime_render_program_t program);

ui_runtime_render_program_unif_t
ui_runtime_render_program_unif_buildin(
    ui_runtime_render_program_t program, ui_runtime_render_program_unif_buildin_t t);

void ui_runtime_render_program_unif_set_buildin(
    ui_runtime_render_program_t program,
    ui_runtime_render_program_unif_buildin_t t, ui_runtime_render_program_unif_t unif);
    
void * ui_runtime_render_program_data(ui_runtime_render_program_t program);
ui_runtime_render_program_t ui_runtime_render_program_from_data(void * data);    

#ifdef __cplusplus
}
#endif

#endif
