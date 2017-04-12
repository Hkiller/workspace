#include "cpe/dr/dr_metalib_manage.h"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/Repository.hpp"

namespace UI { namespace Sprite {

Repository & 
Repository::instance(gd_app_context_t app, const char * name) {
    ui_sprite_repository_t repo = ui_sprite_repository_find_nc(app, name);
    if (repo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "ui_sprite_repository %s not exist!", name ? name : "default");
    }

    return *(Repository*)repo;
}

ComponentMeta & Repository::componentMeta(const char * name) {
    ComponentMeta * e = findComponentMeta(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "componet meta %s not exist!", name);
    }

    return *e;
}

ComponentMeta const & Repository::componentMeta(const char * name) const {
    ComponentMeta const * e = findComponentMeta(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "componet meta %s not exist!", name);
    }

    return *e;
}

ComponentMeta & Repository::createComponentMeta(const char * name, size_t size) {
    ui_sprite_component_meta_t meta = ui_sprite_component_meta_create(*this, name, size);

    if (meta == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "create componet meta %s(size=%d) fail!", name, (int)size);
    }

    return *(ComponentMeta*)meta;
}

void Repository::removeComponentMeta(const char * name) {
    if (ui_sprite_component_meta_t meta = ui_sprite_component_meta_find(*this, name)) {
        ui_sprite_component_meta_free(meta);
    }
}

void Repository::registerEvent(LPDRMETA meta) {
    if (ui_sprite_repository_register_event(*this, meta) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "register event %s fail!", dr_meta_name(meta));
    }
}

void Repository::registerEventsByPrefix(LPDRMETALIB metalib, const char * prefix) {
    if (ui_sprite_repository_register_events_by_prefix(*this, metalib, prefix) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "register events by prefix  %s fail!", prefix);
    }
}

}}
