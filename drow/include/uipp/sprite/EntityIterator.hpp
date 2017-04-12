#ifndef UIPP_SPRITE_ENTITY_ITERATOR_H
#define UIPP_SPRITE_ENTITY_ITERATOR_H
#include "cpepp/utils/ClassCategory.hpp"
#include "System.hpp"

namespace UI { namespace Sprite {

class EntityIterator : public Cpe::Utils::Noncopyable {
public:
    explicit EntityIterator(ui_sprite_entity_it_t it) : m_it(it) {}
    ~EntityIterator();

    Entity * next(void);

private:
    ui_sprite_entity_it_t m_it;
};

}}

#endif
