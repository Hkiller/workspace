#ifndef UI_RUNTIME_RENDER_TECHNIQUE_H
#define UI_RUNTIME_RENDER_TECHNIQUE_H
#include "ui_runtime_module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_technique_it {
    ui_runtime_render_technique_t (*next)(ui_runtime_render_technique_it_t it);
    char m_data[64];
};
    
ui_runtime_render_technique_t ui_runtime_render_technique_create(ui_runtime_render_material_t material);
ui_runtime_render_technique_t ui_runtime_render_technique_clone(ui_runtime_render_material_t material, ui_runtime_render_technique_t technique);
void ui_runtime_render_technique_free(ui_runtime_render_technique_t technique);

ui_runtime_render_material_t ui_runtime_render_technique_material(ui_runtime_render_technique_t technique);
    
ui_runtime_render_state_t ui_runtime_render_technique_check_create_render_state(ui_runtime_render_technique_t technique);
int ui_runtime_render_technique_set_render_state(ui_runtime_render_technique_t technique, ui_runtime_render_state_data_t render_state_data);
ui_runtime_render_state_t ui_runtime_render_technique_render_state(ui_runtime_render_technique_t technique);
    
void ui_runtime_render_technique_set_parent_render_state(
    ui_runtime_render_technique_t technique, ui_runtime_render_state_t render_state);

uint8_t ui_runtime_render_technique_pass_count(ui_runtime_render_technique_t technique);
void ui_runtime_render_technique_passes(ui_runtime_render_technique_t technique, ui_runtime_render_pass_it_t it);
    
#define ui_runtime_render_technique_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
