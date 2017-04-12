#ifndef UI_RUNTIME_RENDER_PASS_H
#define UI_RUNTIME_RENDER_PASS_H
#include "ui_runtime_module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_pass_it {
    ui_runtime_render_pass_t (*next)(ui_runtime_render_pass_it_t it);
    char m_data[64];
};

ui_runtime_render_pass_t ui_runtime_render_pass_create(ui_runtime_render_technique_t technique);
ui_runtime_render_pass_t ui_runtime_render_pass_clone(ui_runtime_render_technique_t technique, ui_runtime_render_pass_t proto);
void ui_runtime_render_pass_free(ui_runtime_render_pass_t pass);

ui_runtime_render_technique_t ui_runtime_render_pass_technique(ui_runtime_render_pass_t pass);
ui_runtime_render_t ui_runtime_render_pass_render(ui_runtime_render_pass_t pass);
    
int ui_runtime_render_pass_set_render_state(ui_runtime_render_pass_t pass, ui_runtime_render_state_data_t render_state_data);
ui_runtime_render_state_t ui_runtime_render_pass_render_state(ui_runtime_render_pass_t pass);
ui_runtime_render_state_t ui_runtime_render_pass_render_state_check_create(ui_runtime_render_pass_t pass);
    
void ui_runtime_render_pass_set_program_state(ui_runtime_render_pass_t pass, ui_runtime_render_program_state_t program_state);
ui_runtime_render_program_state_t ui_runtime_render_pass_program_state(ui_runtime_render_pass_t pass);

uint8_t ui_runtime_render_pass_compatible(ui_runtime_render_pass_t l, ui_runtime_render_pass_t r, uint32_t flags);
    
#define ui_runtime_render_pass_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
