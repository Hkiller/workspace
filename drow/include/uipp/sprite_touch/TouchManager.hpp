#ifndef UIPP_SPRITE_TOUCH_MANAGER_H
#define UIPP_SPRITE_TOUCH_MANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gdpp/app/System.hpp"
#include "ui/sprite_touch/ui_sprite_touch_mgr.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Touch {

class TouchManager : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_touch_mgr_t (void) const { return (ui_sprite_touch_mgr_t)this; }

    const char * name(void) const { return ui_sprite_touch_mgr_name(*this); }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)ui_sprite_touch_mgr_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application const *)ui_sprite_touch_mgr_app(*this); }

    static TouchManager & instance(Gd::App::Application & app, const char * name = NULL);
};

}}}

#endif
