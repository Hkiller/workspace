#ifndef UI_SPRITE_UI_UTILS_H
#define UI_SPRITE_UI_UTILS_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_page_t
ui_sprite_ui_find_page_from_action(
    ui_sprite_ui_module_t module, ui_sprite_fsm_action_t fsm_action, const char * name);
    
plugin_ui_control_t
ui_sprite_ui_find_control_from_action(
    ui_sprite_ui_module_t module, ui_sprite_fsm_action_t fsm_action, const char * path);

#ifdef __cplusplus
}
#endif

#endif
