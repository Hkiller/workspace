#include <memory>
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/Group.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Repository.hpp"

namespace UI { namespace Sprite {

void Group::addElement(Group & element) {
    if (ui_sprite_group_add_group(*this, element) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "group %d(%s): add group %d(%s) fail!",
            this->id(), this->name(),
            element.id(), element.name());
    }
}

void Group::addElement(Entity & element) {
    if (ui_sprite_group_add_entity(*this, element) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "group %d(%s): add entity %d(%s) fail!",
            this->id(), this->name(),
            element.id(), element.name());
    }
}

Entity & Group::firstEntity(void) {
    Entity * e = findFirstEntity();
    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "group %d(%s): get first entity fail!",
            this->id(), this->name());
    }

    return *e;
}

Entity const & Group::firstEntity(void) const {
    Entity const * e = findFirstEntity();
    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "group %d(%s): get first entity fail!",
            this->id(), this->name());
    }

    return *e;
}

::std::auto_ptr<EntityIterator>
Group::entities(mem_allocrator_t alloc) {
    ui_sprite_entity_it_t it = ui_sprite_group_entities(alloc, *this);
    return ::std::auto_ptr<EntityIterator>(new EntityIterator(it));
}

}}
