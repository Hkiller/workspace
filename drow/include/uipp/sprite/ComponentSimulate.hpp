#ifndef UIPP_SPRITE_COMPONENT_SIMULATE_H
#define UIPP_SPRITE_COMPONENT_SIMULATE_H
#include "Component.hpp"

namespace UI { namespace Sprite {

class ComponentSimulate : public Cpe::Utils::SimulateObject {
public:
    Component & component(void) { return *(Component*)ui_sprite_component_from_data((void*)this); }
    Component const & component(void) const { return *(Component*)ui_sprite_component_from_data((void*)this); }

    Entity & entity(void) { return component().entity(); }
    Entity const & entity(void) const { return component().entity(); }

    World & world(void) { return component().world(); }
    World const & world(void) const { return component().world(); }
};

}}

#endif

