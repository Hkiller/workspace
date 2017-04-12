#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Repository.hpp"

namespace UI { namespace Sprite {

void Entity::enter(void) {
    if (ui_sprite_entity_enter(*this) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): enter fail!", this->name(), id());
    }
}

Component & Entity::component(const char * name) {
    Component * e = findComponent(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): componet %s not exist!", this->name(), id(), name);
    }

    return *e;
}

Component const & Entity::component(const char * name) const {
    Component const * e = findComponent(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): componet %s not exist!", this->name(), id(), name);
    }

    return *e;
}

Component & Entity::createComponent(const char * name) {
    ui_sprite_component_t component = ui_sprite_component_create(*this, name);

    if (component == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): create componet %s fail!", this->name(), id(), name);
    }

    return *(Component*)component;
}

void Entity::removeComponent(const char * name) {
    ui_sprite_component_t component = ui_sprite_component_find(*this, name);
    if (component) {
        ui_sprite_component_free(component);
    }
}

Gd::App::Application & Entity::app(void) {
    return world().app();
}
    
Gd::App::Application const & Entity::app(void) const {
    return world().app();
}

Cpe::Dr::DataElement Entity::attr(const char * path) {
    struct dr_data_entry buff;
    dr_data_entry_t a = ui_sprite_entity_find_attr(&buff, *this, path);
    if (a == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): attr %s not exist!", this->name(), id(), path);
    }

    return *a;
}

Cpe::Dr::ConstDataElement Entity::attr(const char * path) const {
    struct dr_data_entry buff;
    dr_data_entry_t a = ui_sprite_entity_find_attr(&buff, *this, path);
    if (a == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): attr %s not exist!", this->name(), id(), path);
    }

    return *a;
}

void Entity::setAttr(const char * attrName, int8_t v) {
    if (ui_sprite_entity_set_attr_int8(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to %d fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, uint8_t v) {
    if (ui_sprite_entity_set_attr_uint8(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to %d fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, int16_t v) {
    if (ui_sprite_entity_set_attr_int16(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to %d fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, uint16_t v) {
    if (ui_sprite_entity_set_attr_uint16(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to %d fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, int32_t v) {
    if (ui_sprite_entity_set_attr_uint32(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to %d fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, uint32_t v) {
    if (ui_sprite_entity_set_attr_uint32(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to " FMT_UINT32_T " fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, int64_t v) {
    if (ui_sprite_entity_set_attr_int64(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to " FMT_INT64_T " fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, uint64_t v) {
    if (ui_sprite_entity_set_attr_uint64(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to " FMT_UINT64_T " fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, float v) {
    if (ui_sprite_entity_set_attr_float(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to %f fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, double v) {
    if (ui_sprite_entity_set_attr_double(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to %f fail!", this->name(), id(), attrName, v);
    }
}

void Entity::setAttr(const char * attrName, const char * v) {
    if (ui_sprite_entity_set_attr_string(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().repository().app(), ::std::runtime_error,
            "entity %s(%d): set attr %s to %s fail!", this->name(), id(), attrName, v);
    }
}

    
void Entity::notifyAttrUpdated(const char * attrs) {
    ui_sprite_entity_notify_attr_updated(*this, attrs);
}

}}
