#ifndef UI_SPRITE_SPINE_ACTION_CONTROL_ENTITY_I_H
#define UI_SPRITE_SPINE_ACTION_CONTROL_ENTITY_I_H
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui/sprite_spine/ui_sprite_spine_control_entity.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_control_entity {
    ui_sprite_spine_module_t m_module;
    char * m_cfg_obj_name;
    char * m_cfg_prefix;
    uint8_t m_cfg_debug;

    ui_sprite_render_anim_t m_render_anim;
    ui_sprite_spine_control_entity_slot_list_t m_slots;
};

enum ui_sprite_spine_control_entity_slot_mode {
    ui_sprite_spine_control_entity_slot_mode_attach,
    ui_sprite_spine_control_entity_slot_mode_bind,
    ui_sprite_spine_control_entity_slot_mode_flush,
};

struct ui_sprite_spine_control_entity_slot {
    ui_sprite_spine_control_entity_t m_owner;
    TAILQ_ENTRY(ui_sprite_spine_control_entity_slot) m_next;
    struct spSlot * m_slot;
    enum ui_sprite_spine_control_entity_slot_mode m_mode;
    uint8_t m_is_active;
    char m_name[32];
    char * m_setups;
    ui_sprite_spine_controled_obj_list_t m_controled_objs;
};

int ui_sprite_spine_control_entity_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_control_entity_unregist(ui_sprite_spine_module_t module);

ui_sprite_spine_control_entity_slot_t
ui_sprite_spine_control_entity_slot_create(
    ui_sprite_spine_control_entity_t control_entity,
    struct spSlot * slot,
    enum ui_sprite_spine_control_entity_slot_mode mode,
    const char * name);

void ui_sprite_spine_control_entity_slot_free(ui_sprite_spine_control_entity_slot_t slot);

void ui_sprite_spine_control_entity_slot_update(
    ui_sprite_entity_t entity, ui_sprite_spine_control_entity_slot_t slot, ui_transform_t local_transform);
    
void ui_sprite_spine_control_entity_slot_real_free(ui_sprite_spine_control_entity_slot_t slot);

#ifdef __cplusplus
}
#endif

#endif
