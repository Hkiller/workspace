#ifndef UI_SPRITE_CHIPMUNK_OBJ_UPDATOR_I_H
#define UI_SPRITE_CHIPMUNK_OBJ_UPDATOR_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_updator.h"
#include "ui_sprite_chipmunk_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_obj_updator {
    ui_sprite_chipmunk_obj_t m_obj;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_updator) m_next_for_obj;
    ui_sprite_chipmunk_obj_updateor_update_fun_t m_update_fun;
    ui_sprite_chipmunk_obj_updateor_clean_fun_t m_clean_fun;
    size_t m_data_capacity;
};

#ifdef __cplusplus
}
#endif

#endif
