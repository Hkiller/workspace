#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/Repository.hpp"

namespace UI { namespace Sprite { namespace Fsm {

Repository & 
Repository::instance(gd_app_context_t app, const char * name) {
    ui_sprite_fsm_module_t repo = ui_sprite_fsm_module_find_nc(app, name);
    if (repo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "ui_sprite_fsm_module %s not exist!", name ? name : "default");
    }

    return *(Repository*)repo;
}

ActionMeta & Repository::actionMeta(const char * name) {
    ActionMeta * e = findActionMeta(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "componet meta %s not exist!", name);
    }

    return *e;
}

ActionMeta const & Repository::actionMeta(const char * name) const {
    ActionMeta const * e = findActionMeta(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "componet meta %s not exist!", name);
    }

    return *e;
}

ActionMeta & Repository::createActionMeta(const char * name, size_t size) {
    ui_sprite_fsm_action_meta_t meta = ui_sprite_fsm_action_meta_create(*this, name, size);

    if (meta == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "create componet meta %s(size=%d) fail!", name, (int)size);
    }

    return *(ActionMeta*)meta;
}

void Repository::removeActionMeta(const char * name) {
    if (ui_sprite_fsm_action_meta_t meta = ui_sprite_fsm_action_meta_find(*this, name)) {
        ui_sprite_fsm_action_meta_free(meta);
    }
}

}}}
