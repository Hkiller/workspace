#include "ui/sprite/ui_sprite_entity.h"
#include "uipp/sprite/EntityIterator.hpp"

namespace UI { namespace Sprite {

Entity * EntityIterator::next(void) {
    if (m_it == NULL) return NULL;

    Entity * r = (Entity *)ui_sprite_entity_it_next(m_it);
    if (r == NULL) {
        ui_sprite_entity_it_free(m_it);
        m_it = NULL;
    }

    return r;
}

EntityIterator::~EntityIterator() {
    if (m_it) ui_sprite_entity_it_free(m_it);
    m_it = NULL;
}

}}
