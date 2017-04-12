#ifndef UI_SPRITE_TOUCH_TOUCHABLE_H
#define UI_SPRITE_TOUCH_TOUCHABLE_H
#include "ui_sprite_touch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_TOUCH_TOUCHABLE_NAME;

ui_sprite_touch_touchable_t ui_sprite_touch_touchable_create(ui_sprite_entity_t entity);
ui_sprite_touch_touchable_t ui_sprite_touch_touchable_find(ui_sprite_entity_t entity);
void ui_sprite_touch_touchable_free(ui_sprite_touch_touchable_t touch_touchable);
void ui_sprite_touch_touchable_set_debug(ui_sprite_touch_touchable_t touch_touchable, uint8_t is_debug);

void ui_sprite_touch_touchable_set_z(ui_sprite_touch_touchable_t touch_touchable, float z);
float ui_sprite_touch_touchable_z(ui_sprite_touch_touchable_t touch_touchable);

/*responser*/

#ifdef __cplusplus
}
#endif

#endif
