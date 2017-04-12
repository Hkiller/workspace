#ifndef UI_SPRITE_UI_NAVIGATION_H
#define UI_SPRITE_UI_NAVIGATION_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_ui_navigation_t ui_sprite_ui_navigation_cast(plugin_ui_navigation_t b_navigation);
    
const char * ui_sprite_ui_navigation_event(ui_sprite_ui_navigation_t navigation);
int ui_sprite_ui_navigation_set_event(ui_sprite_ui_navigation_t navigation, const char * event);

#ifdef __cplusplus
}
#endif

#endif
