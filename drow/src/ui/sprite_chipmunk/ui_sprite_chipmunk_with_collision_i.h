#ifndef UI_SPRITE_CHIPMUNK_WITH_COLLISION_I_H
#define UI_SPRITE_CHIPMUNK_WITH_COLLISION_I_H
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_collision.h"
#include "ui_sprite_chipmunk_with_collision_src_i.h"
#include "ui_sprite_chipmunk_with_collision_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_with_collision {
    ui_sprite_chipmunk_module_t m_module;
    ui_sprite_chipmunk_with_collision_src_list_t m_srcs;
    ui_sprite_chipmunk_with_collision_body_list_t m_bodies;
};

int ui_sprite_chipmunk_with_collision_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_with_collision_unregist(ui_sprite_chipmunk_module_t module);

ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_collision_load(
    void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
