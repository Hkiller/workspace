#ifndef UI_RUNTIME_RENDER_CMD_H
#define UI_RUNTIME_RENDER_CMD_H
#include "ui_runtime_render_buff_use.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_runtime_render_cmd_type {
    ui_runtime_render_cmd_triangles = 1,
    ui_runtime_render_cmd_mesh,
    ui_runtime_render_cmd_primitive,
    ui_runtime_render_cmd_group,
} ui_runtime_render_cmd_type_t;

void ui_runtime_render_cmd_free(ui_runtime_render_cmd_t cmd);

ui_runtime_render_cmd_type_t ui_runtime_render_cmd_type(ui_runtime_render_cmd_t cmd);
const char * ui_runtime_render_cmd_type_str(ui_runtime_render_cmd_t cmd);
ui_runtime_render_queue_group_t ui_runtime_render_cmd_queue_group(ui_runtime_render_cmd_t cmd);

uint8_t ui_runtime_render_cmd_skip_batch(ui_runtime_render_cmd_t cmd);
void ui_runtime_render_cmd_set_skip_batch(ui_runtime_render_cmd_t cmd, uint8_t skip_batch);
    
void ui_runtime_render_cmd_set_3d(ui_runtime_render_cmd_t cmd, uint8_t is_3d, uint8_t is_transparent);

ui_runtime_render_material_t ui_runtime_render_cmd_material(ui_runtime_render_cmd_t cmd);
ui_runtime_render_state_t ui_runtime_render_cmd_render_state(ui_runtime_render_cmd_t cmd);

ui_runtime_render_buff_use_t ui_runtime_render_cmd_vertexs(ui_runtime_render_cmd_t cmd);
uint32_t ui_runtime_render_cmd_vertex_count(ui_runtime_render_cmd_t cmd);
void const * ui_runtime_render_cmd_vertex_data(ui_runtime_render_cmd_t cmd);
ui_runtime_render_buff_type_t ui_runtime_render_cmd_vertex_e_type(ui_runtime_render_cmd_t cmd);
    
ui_runtime_render_buff_use_t ui_runtime_render_cmd_indexes(ui_runtime_render_cmd_t cmd);
uint32_t ui_runtime_render_cmd_index_count(ui_runtime_render_cmd_t cmd);
void const * ui_runtime_render_cmd_index_data(ui_runtime_render_cmd_t cmd);
ui_runtime_render_buff_type_t ui_runtime_render_cmd_index_e_type(ui_runtime_render_cmd_t cmd);

const char * ui_runtime_render_cmd_type_to_str(ui_runtime_render_cmd_type_t cmd_type);
    
#ifdef __cplusplus
}
#endif

#endif
