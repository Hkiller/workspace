#ifndef UI_SPRITE_BARRAGE_OBJ_H
#define UI_SPRITE_BARRAGE_OBJ_H
#include "cpe/cfg/cfg_types.h"
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BARRAGE_OBJ_NAME;

ui_sprite_barrage_obj_t ui_sprite_barrage_obj_create(ui_sprite_entity_t entity);
ui_sprite_barrage_obj_t ui_sprite_barrage_obj_find(ui_sprite_entity_t entity);
void ui_sprite_barrage_obj_free(ui_sprite_barrage_obj_t barrage_obj);

void ui_sprite_barrage_obj_set_target_fun(
    ui_sprite_barrage_obj_t barrage_obj, plugin_barrage_target_fun_t fun, void * ctx);

void ui_sprite_barrage_obj_pause_barrages(
    ui_sprite_barrage_obj_t barrage_obj, const char * group_name);
void ui_sprite_barrage_obj_resume_barrages(
    ui_sprite_barrage_obj_t barrage_obj, const char * group_name);

void ui_sprite_barrage_obj_set_mask(ui_sprite_barrage_obj_t barrage_obj, const char * emitter_perfix, uint32_t mask);
uint32_t ui_sprite_barrage_obj_mask(ui_sprite_barrage_obj_t barrage_obj);

uint8_t ui_sprite_barrage_obj_is_barrages_enable(
    ui_sprite_barrage_obj_t barrage_obj, const char * group_name);

void ui_sprite_barrage_obj_enable_barrages(
    ui_sprite_barrage_obj_t barrage_obj, const char * group_name, ui_sprite_event_t collision_event, uint32_t loop_count);
void ui_sprite_barrage_obj_disable_barrages(
    ui_sprite_barrage_obj_t barrage_obj, const char * group_name, uint8_t destory_bullets);

void ui_sprite_barrage_obj_clear_bullets(
    ui_sprite_barrage_obj_t barrage_obj, const char * group_name);
    
uint8_t ui_sprite_barrage_obj_sync_barrages(ui_sprite_barrage_obj_t barrage_obj, const char * group_name);

#ifdef __cplusplus
}
#endif

#endif
