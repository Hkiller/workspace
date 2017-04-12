#ifndef UI_SPRITE_SPINE_CONTROLED_OBJ_I_H
#define UI_SPRITE_SPINE_CONTROLED_OBJ_I_H
#include "ui/sprite_spine/ui_sprite_spine_controled_obj.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_controled_obj {
    ui_sprite_spine_module_t m_module;
    ui_sprite_spine_control_entity_slot_t m_slot;
    TAILQ_ENTRY(ui_sprite_spine_controled_obj) m_next;
    uint8_t m_is_binding;
};

int ui_sprite_spine_controled_obj_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_controled_obj_unregist(ui_sprite_spine_module_t module);

void ui_sprite_spine_controled_obj_set_slot(ui_sprite_spine_controled_obj_t obj, ui_sprite_spine_control_entity_slot_t slot);
    
#ifdef __cplusplus
}
#endif

#endif
