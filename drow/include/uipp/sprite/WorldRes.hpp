#ifndef UIPP_SPRITE_WORLDRES_H
#define UIPP_SPRITE_WORLDRES_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite/ui_sprite_world_res.h"
#include "System.hpp"

namespace UI { namespace Sprite {

class WorldRes : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_world_res_t () const { return (ui_sprite_world_res_t)this; }

    World & world(void) { return *(World*)ui_sprite_world_res_world(*this); }
    World const & world(void) const { return *(World const *)ui_sprite_world_res_world(*this); }

    void * data(void) { return ui_sprite_world_res_data(*this); }
    void const * data(void) const { return ui_sprite_world_res_data(*this); }
    size_t dataSize(void) const { return ui_sprite_world_res_data_size(*this); }

    template<typename T>
    T & as(void) { return *(T*)data(); }

    template<typename T>
    T const & as(void) const { return *(T const *)data(); }
};

}}

#endif
