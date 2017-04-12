#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "uipp/sprite_touch/TouchManager.hpp"

namespace UI { namespace Sprite { namespace Touch {

TouchManager & TouchManager::instance(Gd::App::Application & app, const char * name) {
    ui_sprite_touch_mgr_t mgr = ui_sprite_touch_mgr_find_nc(app, name);
    if (mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "ui_sprite_touch_mgr %s not exist!", name ? name : "default");
    }

    return *(TouchManager*)mgr;
}

}}}



