#ifndef UI_SPRITE_WORLD_DATA_I_H
#define UI_SPRITE_WORLD_DATA_I_H
#include "cpe/dr/dr_types.h"
#include "ui_sprite_world_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_world_data {
    ui_sprite_world_t m_world;
    TAILQ_ENTRY(ui_sprite_world_data) m_next;
    size_t m_capacity;
    struct dr_data m_data;
};

void ui_sprite_world_data_free_all(const ui_sprite_world_t world);

#ifdef __cplusplus
}
#endif

#endif
