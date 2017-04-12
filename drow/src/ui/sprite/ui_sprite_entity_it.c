#include "ui_sprite_entity_it_i.h"

ui_sprite_entity_t ui_sprite_entity_it_next(ui_sprite_entity_it_t entity_it) {
    return entity_it->m_next(entity_it);
}

void ui_sprite_entity_it_free(ui_sprite_entity_it_t entity_it) {
    entity_it->m_free(entity_it);
}

