#ifndef UIPP_SPRITE_COMPONENT_META_H
#define UIPP_SPRITE_COMPONENT_META_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "System.hpp"

namespace UI { namespace Sprite {

class ComponentMeta : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_component_meta_t () const { return (ui_sprite_component_meta_t)this; }
};

}}

#endif
