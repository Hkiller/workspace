#ifndef UI_SPRITE_GROUP_I_H
#define UI_SPRITE_GROUP_I_H
#include "ui/sprite/ui_sprite_group.h"
#include "ui_sprite_world_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_group {
    uint32_t m_id;
    const char * m_name;
    ui_sprite_world_t m_world;
    ui_sprite_group_binding_list_t m_join_groups;
    ui_sprite_group_binding_list_t m_elements;
    struct cpe_hash_entry m_hh_for_id;
    struct cpe_hash_entry m_hh_for_name;
};

uint32_t ui_sprite_group_id_hash(const ui_sprite_group_t group);
int ui_sprite_group_id_eq(const ui_sprite_group_t l, const ui_sprite_group_t r);

uint32_t ui_sprite_group_name_hash(const ui_sprite_group_t group);
int ui_sprite_group_name_eq(const ui_sprite_group_t l, const ui_sprite_group_t r);

void ui_sprite_group_free_all(ui_sprite_world_t world);

#ifdef __cplusplus
}
#endif

#endif
