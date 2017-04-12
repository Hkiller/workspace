#ifndef UI_RUNTIME_SOUND_PLAYING_H
#define UI_RUNTIME_SOUND_PLAYING_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_sound_playing_t ui_runtime_sound_playing_create(ui_runtime_sound_chanel_t chanel, ui_cache_res_t res, uint8_t is_loop);
void ui_runtime_sound_playing_free(ui_runtime_sound_playing_t playing);

ui_runtime_sound_chanel_t ui_runtime_sound_playing_chanel(ui_runtime_sound_playing_t playing);    
uint32_t ui_runtime_sound_playing_id(ui_runtime_sound_playing_t playing);
ui_cache_res_t ui_runtime_sound_playing_res(ui_runtime_sound_playing_t playing);
uint8_t ui_runtime_sound_playing_is_loop(ui_runtime_sound_playing_t playing);

float ui_runtime_sound_playing_volum(ui_runtime_sound_playing_t playing);
void ui_runtime_sound_playing_set_volumn(ui_runtime_sound_playing_t playing, float volume);

ui_runtime_sound_playing_t ui_runtime_sound_playing_find_by_id(ui_runtime_module_t module, uint32_t id);

#ifdef __cplusplus
}
#endif

#endif 
