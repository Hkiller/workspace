#ifndef UI_SPRITE_CHIPMUNK_ON_COLLISION_I_H
#define UI_SPRITE_CHIPMUNK_ON_COLLISION_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_on_collision.h"
#include "ui_sprite_chipmunk_module_i.h"
#include "ui_sprite_chipmunk_monitor_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_on_collision {
    ui_sprite_chipmunk_module_t m_module;
    struct ui_sprite_chipmunk_monitor m_monitor;
    char * m_on_collision_begin;
    char * m_on_collision_end;
};

int ui_sprite_chipmunk_on_collision_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_on_collision_unregist(ui_sprite_chipmunk_module_t module);

#ifdef __cplusplus
}
#endif

#endif
