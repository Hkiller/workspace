#ifndef UIPP_SPRITE_WORLDRES_GEN_H
#define UIPP_SPRITE_WORLDRES_GEN_H
#include "WorldRes.hpp"
#include "World.hpp"

namespace UI { namespace Sprite {

template<typename T, typename OuterT> 
class WorldResGen : public T {
public:
    static void free_fun(ui_sprite_world_res_t world_res, void * ctx) {
        ((OuterT*)ui_sprite_world_res_data(world_res))->~OuterT();
    }

    WorldResGen(WorldRes & world_res) : m_world_res(world_res) {
        ui_sprite_world_res_set_free_fun(world_res, free_fun, NULL);
    }

    static OuterT & install(UI::Sprite::World & world) {
        WorldRes & world_res = world.createRes(OuterT::NAME, sizeof(OuterT));
        return * new (world_res.data()) OuterT(world_res);
    }

    template<typename ModuleT>
    static OuterT & install(ModuleT & module, UI::Sprite::World & world) {
        WorldRes & world_res = world.createRes(OuterT::NAME, sizeof(OuterT));
        return * new (world_res.data()) OuterT(module, world_res);
    }

    WorldRes & worldRes(void) { return m_world_res; }
    WorldRes const & worldRes(void) const { return m_world_res; }

    UI::Sprite::World & world(void) { return m_world_res.world(); }
    UI::Sprite::World const & world(void) const { return m_world_res.world(); }

    void destory(void) { world().template removeRes<OuterT>(); }

private:
    WorldRes & m_world_res;
};

}}

#endif
