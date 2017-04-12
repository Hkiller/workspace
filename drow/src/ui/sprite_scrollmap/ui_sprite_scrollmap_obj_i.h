#ifndef UI_SPRITE_SCROLLMAP_OBJ_I_H
#define UI_SPRITE_SCROLLMAP_OBJ_I_H
#include "ui/sprite_scrollmap/ui_sprite_scrollmap_obj.h"
#include "ui_sprite_scrollmap_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_scrollmap_obj {
    ui_sprite_scrollmap_env_t m_env;
    plugin_scrollmap_obj_t m_obj;
    uint8_t m_is_suspend;
};

struct ui_sprite_scrollmap_obj_stub {
    ui_sprite_scrollmap_obj_t m_sprite_obj;
};

int ui_sprite_scrollmap_obj_regist(ui_sprite_scrollmap_module_t module);
void ui_sprite_scrollmap_obj_unregist(ui_sprite_scrollmap_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
