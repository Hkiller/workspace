#ifndef UI_RUNTIME_RENDER_VERTEX_BUFF_H
#define UI_RUNTIME_RENDER_VERTEX_BUFF_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_vertex_buff_t
ui_runtime_render_vertex_buff_create(ui_runtime_render_t render, ui_runtime_render_buff_type_t type);

void ui_runtime_render_vertex_buff_free(ui_runtime_render_vertex_buff_t buff);
    
ui_runtime_render_buff_type_t ui_runtime_render_vertex_buff_type(ui_runtime_render_vertex_buff_t buff);
    
#ifdef __cplusplus
}
#endif

#endif
