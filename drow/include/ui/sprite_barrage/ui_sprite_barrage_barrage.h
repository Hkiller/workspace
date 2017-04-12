#ifndef UI_SPRITE_BARRAGE_BARRAGE_H
#define UI_SPRITE_BARRAGE_BARRAGE_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_barrage_barrage_t
ui_sprite_barrage_barrage_create(
    ui_sprite_barrage_obj_t barrage_obj,
    const char * part_name, const char * group,
    const char * res, plugin_barrage_data_emitter_flip_type_t flip_type);
    
void ui_sprite_barrage_barrage_free(ui_sprite_barrage_barrage_t emitter);
void ui_sprite_barrage_barrage_free_group(ui_sprite_barrage_obj_t barrage_obj, const char * group);
void ui_sprite_barrage_barrage_free_all(ui_sprite_barrage_obj_t barrage_obj);

plugin_barrage_emitter_t ui_sprite_barrage_barrage_emitter(ui_sprite_barrage_barrage_t emitter);

void ui_sprite_barrage_barrage_set_transform(ui_sprite_barrage_barrage_t emitter, ui_transform_t transform);

#ifdef __cplusplus
}
#endif

#endif
