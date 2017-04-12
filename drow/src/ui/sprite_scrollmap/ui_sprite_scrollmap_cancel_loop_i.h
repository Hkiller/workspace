#ifndef UI_SPRITE_SCROLLMAP_CANCEL_LOOP_I_H
#define UI_SPRITE_SCROLLMAP_CANCEL_LOOP_I_H
#include "ui/sprite_scrollmap/ui_sprite_scrollmap_cancel_loop.h"
#include "ui_sprite_scrollmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_scrollmap_cancel_loop {
    ui_sprite_scrollmap_module_t m_module;
    char m_layer_prefix[64];
};

int ui_sprite_scrollmap_cancel_loop_regist(ui_sprite_scrollmap_module_t module);
void ui_sprite_scrollmap_cancel_loop_unregist(ui_sprite_scrollmap_module_t module);

#ifdef __cplusplus
}
#endif

#endif
