#ifndef UI_RUNTIME_SOUND_CHANEL_H
#define UI_RUNTIME_SOUND_CHANEL_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_sound_chanel_t ui_runtime_sound_chanel_create(ui_runtime_sound_group_t group, ui_runtime_sound_backend_t backend);
void ui_runtime_sound_chanel_free(ui_runtime_sound_chanel_t chanel);

ui_runtime_sound_chanel_t ui_runtime_sound_chanel_find_by_name(ui_runtime_module_t module, const char * name);

ui_runtime_sound_chanel_state_t ui_runtime_sound_chanel_state(ui_runtime_sound_chanel_t chanel);
const char * ui_runtime_sound_chanel_state_str(ui_runtime_sound_chanel_t chanel);

void * ui_runtime_sound_chanel_data(ui_runtime_sound_chanel_t chanel);

#ifdef __cplusplus
}
#endif

#endif 
