#ifndef UI_SPRITE_EVENT_META_I_H
#define UI_SPRITE_EVENT_META_I_H
#include "cpe/utils/hash.h"
#include "ui/sprite/ui_sprite_types.h"
#include "ui_sprite_repository_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_event_meta {
    const char * m_name;
    LPDRMETA m_meta;
    uint8_t m_debug_level;
    struct cpe_hash_entry m_hh_for_repo;
};

void ui_sprite_event_meta_free_all(ui_sprite_repository_t repo);

ui_sprite_event_meta_t ui_sprite_event_meta_find(ui_sprite_repository_t repo, const char * name);
uint8_t ui_sprite_repository_event_debug_level(ui_sprite_repository_t repo, LPDRMETA meta);
int ui_sprite_repository_register_event_debug_level(ui_sprite_repository_t repo, const char * name, uint8_t level);

int ui_sprite_event_meta_build_event(
    ui_sprite_event_t event,
    ui_sprite_repository_t repo, ui_sprite_world_t world, ui_sprite_entity_t entity,
    char * event_args, dr_data_source_t data_source);

uint32_t ui_sprite_event_meta_hash(const ui_sprite_event_meta_t meta);
int ui_sprite_event_meta_eq(const ui_sprite_event_meta_t l, const ui_sprite_event_meta_t r);
 
#ifdef __cplusplus
}
#endif

#endif

