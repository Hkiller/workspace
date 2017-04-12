#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/Component.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Repository.hpp"

namespace UI { namespace Sprite {

void Component::enter(void) {
    if (ui_sprite_component_enter(*this) != 0) {
        APP_CTX_THROW_EXCEPTION(
            entity().world().app(), ::std::runtime_error,
            "component enter fail!");
    }
}

void Component::startUpdate(void) {
    if (ui_sprite_component_start_update(*this) != 0) {
        APP_CTX_THROW_EXCEPTION(
            entity().world().app(), ::std::runtime_error,
            "component start update fail!");
    }
}

void Component::setAttrData(dr_data_t data) {
    if (ui_sprite_component_set_attr_data(*this, data) != 0) {
        APP_CTX_THROW_EXCEPTION(
            entity().world().app(), ::std::runtime_error,
            "component set attr data fail!");
    }
}

}}
