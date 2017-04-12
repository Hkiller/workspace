#ifndef UI_SPRITE_SPINE_MOVE_ENTITY_I_H
#define UI_SPRITE_SPINE_MOVE_ENTITY_I_H
#include "ui/sprite_spine/ui_sprite_spine_move_entity.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_move_entity {
    ui_sprite_spine_module_t m_module;
    char * m_cfg_skeleton_res;
    char * m_cfg_bone;

    ui_vector_2 m_base_pos;
    ui_runtime_render_obj_t m_render_obj;
    struct spBone * m_bone;
};

int ui_sprite_spine_move_entity_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_move_entity_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
