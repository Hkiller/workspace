#ifndef UI_RUNTIME_SOUND_GROUP_H
#define UI_RUNTIME_SOUND_GROUP_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif
        
ui_runtime_sound_group_t ui_runtime_sound_group_create(ui_runtime_module_t module, const char * name, ui_runtime_sound_type_t sound_type);
void ui_runtime_sound_group_free(ui_runtime_sound_group_t group);

ui_runtime_sound_group_t ui_runtime_sound_group_find(ui_runtime_module_t module, const char * name);

uint8_t ui_runtime_sound_group_is_enable(ui_runtime_sound_group_t group);
void ui_runtime_sound_group_set_enable(ui_runtime_sound_group_t group, uint8_t enable);

float ui_runtime_sound_group_volum(ui_runtime_sound_group_t group);
void ui_runtime_sound_group_set_volumn(ui_runtime_sound_group_t group, float volume);
    
ui_runtime_sound_playing_t ui_runtime_sound_group_play(ui_runtime_sound_group_t group, ui_cache_res_t res, uint8_t is_loop);
ui_runtime_sound_playing_t ui_runtime_sound_group_play_by_path(ui_runtime_sound_group_t group, const char * res_path, uint8_t is_loop);

ui_runtime_sound_group_schedule_type_t ui_runtime_sound_group_schedule_type(ui_runtime_sound_group_t group);
void ui_runtime_sound_group_set_schedule_type(ui_runtime_sound_group_t group, ui_runtime_sound_group_schedule_type_t schedule_type);

#ifdef __cplusplus
}
#endif

#endif 
