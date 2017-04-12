#ifndef UIPP_SPRITE_CFG_LOADER_H
#define UIPP_SPRITE_CFG_LOADER_H
#include "gdpp/app/System.hpp"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/WorldRes.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite/Component.hpp"
#include "uipp/sprite_fsm/Fsm.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/Action.hpp"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Cfg {

class CfgLoader : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_cfg_loader_t (void) const { return (ui_sprite_cfg_loader_t)this; }

    const char * name(void) const { return ui_sprite_cfg_loader_name(*this); }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)ui_sprite_cfg_loader_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application const *)ui_sprite_cfg_loader_app(*this); }

    /*world*/
    bool tryLoadWorld(World & world, const char * src_path) { return ui_sprite_cfg_loader_load_world_from_path(*this, world, src_path) == 0 ? true : false; }
    bool tryLoadWorld(World & world, cfg_t cfg) { return ui_sprite_cfg_loader_load_world_from_cfg(*this, world, cfg) == 0 ? true : false; }
    
    void loadWorld(World & world, const char * src_path);
    void loadWorld(World & world, cfg_t cfg);

    /*entity*/
    bool tryLoadEntity(Entity & entity, const char * src_path) { return ui_sprite_cfg_loader_load_entity_from_path(*this, entity, src_path) == 0 ? true : false; }
    bool tryLoadEntity(Entity & entity, cfg_t cfg) { return ui_sprite_cfg_loader_load_entity_from_cfg(*this, entity, cfg) == 0 ? true : false; }
    
    void loadEntity(Entity & entity, const char * src_path);
    void loadEntity(Entity & entity, cfg_t cfg);

    /*fsm*/
    bool tryLoadFsm(Fsm::Fsm & fsm, const char * src_path) { return ui_sprite_cfg_load_fsm_from_path(*this, fsm, src_path) == 0 ? true : false; }
    bool tryLoadFsm(Fsm::Fsm & fsm, cfg_t cfg) { return ui_sprite_cfg_load_fsm(*this, fsm, cfg) == 0 ? true : false; }
    
    void loadFsm(Fsm::Fsm & fsm, const char * src_path);
    void loadFsm(Fsm::Fsm & fsm, cfg_t cfg);
    
    /*resource*/
    void addResourceLoader(const char * name, ui_sprite_cfg_load_resource_fun_t fun, void * ctx);
    void removeResourceLoader(const char * name) { ui_sprite_cfg_loader_remove_resource_loader(*this, name); }

    WorldRes * tryLoadResource(World & world, const char * name, const char * src_path) { 
        return (WorldRes *)ui_sprite_cfg_loader_load_resource_from_path(*this, world, name, src_path);
    }

    WorldRes & loadResource(World & world, const char * name, const char * src_path);

    WorldRes * tryLoadResource(World & world, const char * name, cfg_t cfg) { 
        return (WorldRes *)ui_sprite_cfg_loader_load_resource_from_cfg(*this, world, name, cfg);
    }

    WorldRes & loadResource(World & world, const char * name, cfg_t cfg);
    
    template<typename T>
    T * tryLoadResource(World & world, const char * src_path) {
        WorldRes * r = tryLoadResource(world, T::NAME, src_path);
        return r ? (T*)r->data() : NULL;
    }

    template<typename T>
    T & loadResource(World & world, const char * src_path) {
        return *(T *)loadResource(world, T::NAME, src_path).data();
    }

    template<typename T>
    T * tryLoadResource(World & world, cfg_t cfg) {
        WorldRes * r = tryLoadResource(world, T::NAME, cfg);
        return r ? (T*)r->data() : NULL;
    }

    template<typename T>
    T & loadResource(World & world, cfg_t cfg) {
        return *(T *)loadResource(world, T::NAME, cfg).data();
    }

    /*component*/
    void addComponentLoader(const char * name, ui_sprite_cfg_load_comp_fun_t fun, void * ctx);
    void removeComponentLoader(const char * name) { ui_sprite_cfg_loader_remove_comp_loader(*this, name); }

    void loadComponents(Entity & entity, const char * src_path);
    void loadComponents(Entity & entity, cfg_t cfg);

    Component * tryLoadComponent(Entity & entity, const char * name, const char * src_path) { 
        return (Component *)ui_sprite_cfg_loader_load_component_from_path(*this, entity, name, src_path);
    }

    Component & loadComponent(Entity & entity, const char * name, const char * src_path);

    Component * tryLoadComponent(Entity & entity, const char * name, cfg_t cfg) { 
        return (Component *)ui_sprite_cfg_loader_load_component_from_cfg(*this, entity, name, cfg);
    }

    Component & loadComponent(Entity & entity, const char * name, cfg_t cfg);
    
    template<typename T>
    T * tryLoadComponent(Entity & entity, const char * src_path) {
        Component * r = tryLoadComponent(entity, T::NAME, src_path);
        return r ? (T*)r->data() : NULL;
    }

    template<typename T>
    T & loadComponent(Entity & entity, const char * src_path) {
        return *(T *)loadComponent(entity, T::NAME, src_path).data();
    }

    template<typename T>
    T * tryLoadComponent(Entity & entity, cfg_t cfg) {
        Component * r = tryLoadComponent(entity, T::NAME, cfg);
        return r ? (T*)r->data() : NULL;
    }

    template<typename T>
    T & loadComponent(Entity & entity, cfg_t cfg) {
        return *(T *)loadComponent(entity, T::NAME, cfg).data();
    }

    /*action*/
    void addActionLoader(const char * name, ui_sprite_cfg_load_action_fun_t fun, void * ctx);
    void removeActionLoader(const char * name) { ui_sprite_cfg_loader_remove_action_loader(*this, name); }

    Fsm::Action * tryLoadAction(Fsm::State & state, const char * name, const char * src_path) { 
        return (Fsm::Action *)ui_sprite_cfg_loader_load_action_from_path(*this, state, name, src_path);
    }

    Fsm::Action & loadAction(Fsm::State & state, const char * name, const char * src_path);
    
    Fsm::Action * tryLoadAction(Fsm::State & state, const char * name, cfg_t cfg) { 
        return (Fsm::Action *)ui_sprite_cfg_loader_load_action_from_cfg(*this, state, name, cfg);
    }

    Fsm::Action & loadAction(Fsm::State & state, const char * name, cfg_t cfg);
    
    template<typename T>
    T * tryLoadAction(Fsm::State & state, const char * src_path) {
        Fsm::Action * r = tryLoadAction(state, T::NAME, src_path);
        return r ? (T*)r->data() : NULL;
    }

    template<typename T>
    T & loadAction(Fsm::State & state, const char * src_path) {
        return *(T *)loadAction(state, T::NAME, src_path).data();
    }

    template<typename T>
    T * tryLoadAction(Fsm::State & state, cfg_t cfg) {
        Fsm::Action * r = tryLoadAction(state, T::NAME, cfg);
        return r ? (T*)r->data() : NULL;
    }

    template<typename T>
    T & loadAction(Fsm::State & state, cfg_t cfg) {
        return *(T *)loadAction(state, T::NAME, cfg).data();
    }

    /*proto entity*/

    /*proto fsm*/
    Fsm::Fsm * tryLoadProtoFsm(World & world, const char * name, const char * src_path) {
        return (Fsm::Fsm *)ui_sprite_cfg_loader_load_fsm_proto_from_path(*this, world, name, src_path);
    }

    Fsm::Fsm & loadProtoFsm(World & world, const char * name, const char * src_path);

    Fsm::Fsm * tryLoadProtoFsm(World & world, const char * name, cfg_t cfg) {
        return (Fsm::Fsm *)ui_sprite_cfg_loader_load_fsm_proto_from_cfg(*this, world, name, cfg);
    }

    Fsm::Fsm & loadProtoFsm(World & world, const char * name, cfg_t cfg);

    /*instance */
    static CfgLoader & instance(Gd::App::Application & app, const char * name = NULL);
};

}}}

#endif
