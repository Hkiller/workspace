#ifndef UI_RUNTIME_MODULE_H
#define UI_RUNTIME_MODULE_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "render/model/ui_object_ref.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_module_t
ui_runtime_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr,
    const char * name, error_monitor_t em);

void ui_runtime_module_free(ui_runtime_module_t module);

gd_app_context_t ui_runtime_module_app(ui_runtime_module_t module);
const char * ui_runtime_module_name(ui_runtime_module_t module);

ui_runtime_module_t ui_runtime_module_find(gd_app_context_t app, cpe_hash_string_t name);
ui_runtime_module_t ui_runtime_module_find_nc(gd_app_context_t app, const char * name);

ui_cache_manager_t ui_runtime_module_cache_mgr(ui_runtime_module_t module);
ui_data_mgr_t ui_runtime_module_data_mgr(ui_runtime_module_t module);

ui_data_src_t ui_runtime_module_find_src(ui_runtime_module_t module, UI_OBJECT_SRC_REF const * src_ref, uint8_t src_type);

void ui_runtime_module_update(ui_runtime_module_t module, float delta);

uint32_t ui_runtime_module_obj_count(ui_runtime_module_t module);

ui_runtime_runing_level_t ui_runtime_module_runing_level(ui_runtime_module_t module);
int ui_runtime_module_set_runing_level(ui_runtime_module_t module, ui_runtime_runing_level_t runing_level);    

/*sound*/
uint8_t ui_runtime_module_sound_is_pause(ui_runtime_module_t module);
void ui_runtime_module_sound_set_pause(ui_runtime_module_t module, uint8_t is_pause);
void ui_runtime_module_sound_sync_suspend(ui_runtime_module_t module);
float ui_runtime_module_sound_volum(ui_runtime_module_t module);
void ui_runtime_module_sound_set_volumn(ui_runtime_module_t module, float volume);
ui_runtime_sound_playing_t
ui_runtime_module_sound_play_by_res(ui_runtime_module_t module, const char * group_name, ui_cache_res_t res, uint8_t is_loop);
ui_runtime_sound_playing_t
ui_runtime_module_sound_play_by_res_path(ui_runtime_module_t module, const char * group_name, const char * res_path, uint8_t is_loop);

void ui_runtime_module_sound_stop_by_res(ui_runtime_module_t module, ui_cache_res_t res);
    
#ifdef __cplusplus
}
#endif

#endif
