#ifndef UI_SPRITE_ENTITY_IT_I_H
#define UI_SPRITE_ENTITY_IT_I_H
#include "ui_sprite_entity_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_entity_it {
    ui_sprite_entity_t (*m_next)(ui_sprite_entity_it_t entity_it);
    void (*m_free)(ui_sprite_entity_it_t entity_it);
};

#ifdef __cplusplus
}
#endif

#endif
