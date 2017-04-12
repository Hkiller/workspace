#ifndef UIPP_SPRITE_TOUCH_COMPONENT_H
#define UIPP_SPRITE_TOUCH_COMPONENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite_touch/ui_sprite_touch_touchable.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Touch {

class Touchable : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_touch_touchable_t () const { return (ui_sprite_touch_touchable_t)this; }

    static const char * NAME;
};

}}}

#endif
