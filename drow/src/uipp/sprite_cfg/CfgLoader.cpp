#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_fsm/Fsm.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_cfg/CfgLoader.hpp"

namespace UI { namespace Sprite { namespace Cfg {

/*world*/
void CfgLoader::loadWorld(World & world, const char * src_path) {
    if (!tryLoadWorld(world, src_path)) {
        APP_CTX_THROW_EXCEPTION(
            world.app(),
            ::std::runtime_error,
            "world load from %s fail!", src_path);
    }
}

void CfgLoader::loadWorld(World & world, cfg_t cfg) {
    if (!tryLoadWorld(world, cfg)) {
        APP_CTX_THROW_EXCEPTION(
            world.app(),
            ::std::runtime_error,
            "world load from cfg fail!");
    }
}

/*entity*/
void CfgLoader::loadEntity(Entity & entity, const char * src_path) {
    if (!tryLoadEntity(entity, src_path)) {
        APP_CTX_THROW_EXCEPTION(
            entity.world().app(),
            ::std::runtime_error,
            "entity load from %s fail!", src_path);
    }
}

void CfgLoader::loadEntity(Entity & entity, cfg_t cfg) {
    if (!tryLoadEntity(entity, cfg)) {
        APP_CTX_THROW_EXCEPTION(
            entity.world().app(),
            ::std::runtime_error,
            "entity load from cfg fail!");
    }
}

/*fsm*/
void CfgLoader::loadFsm(Fsm::Fsm & fsm, const char * src_path) {
    if (!tryLoadFsm(fsm, src_path)) {
        APP_CTX_THROW_EXCEPTION(fsm.world().app(), ::std::runtime_error, "fsm load from %s fail!", src_path);
    }
}

void CfgLoader::loadFsm(Fsm::Fsm & fsm, cfg_t cfg) {
    if (!tryLoadFsm(fsm, cfg)) {
        APP_CTX_THROW_EXCEPTION(fsm.world().app(), ::std::runtime_error, "fsm load from cfg fail!");
    }
}

/*resource*/
void CfgLoader::addResourceLoader(const char * name, ui_sprite_cfg_load_resource_fun_t fun, void * ctx) {
    if (ui_sprite_cfg_loader_add_resource_loader(*this, name, fun, ctx) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "add resource load %s fail!", name);
    }
}

WorldRes & CfgLoader::loadResource(World & world, const char * name, const char * src_path) {
    WorldRes * r = tryLoadResource(world, name, src_path);

    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "load resource %s from %s fail!", name, src_path);
    }

    return *r;
}

WorldRes & CfgLoader::loadResource(World & world, const char * name, cfg_t cfg) {
    WorldRes * r = tryLoadResource(world, name, cfg);

    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "load resource %s from cfg fail!", name);
    }

    return *r;
}

/*action*/
void CfgLoader::addActionLoader(const char * name, ui_sprite_cfg_load_action_fun_t fun, void * ctx) {
    if (ui_sprite_cfg_loader_add_action_loader(*this, name, fun, ctx) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "add action load %s fail!", name);
    }
}

Fsm::Action & CfgLoader::loadAction(Fsm::State & state, const char * name, const char * src_path) {
    Fsm::Action * r = tryLoadAction(state, name, src_path);

    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            state.fsm().entity().world().app(),
            ::std::runtime_error,
            "load action %s fail!", name);
    }

    return *r;
}

Fsm::Action & CfgLoader::loadAction(Fsm::State & state, const char * name, cfg_t cfg) {
    Fsm::Action * r = tryLoadAction(state, name, cfg);

    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            state.fsm().entity().world().app(),
            ::std::runtime_error,
            "load action %s fail!", name);
    }

    return *r;
}

/*component*/
void CfgLoader::loadComponents(Entity & entity, const char * src_path) {
    if (ui_sprite_cfg_loader_load_components_from_path(*this, entity, src_path) != 0) {
        APP_CTX_THROW_EXCEPTION(
            entity.world().app(),
            ::std::runtime_error,
            "entity %d(%s) load components from %s fail!", entity.id(), entity.name(), src_path);
    }
}

void CfgLoader::loadComponents(Entity & entity, cfg_t cfg) {
    if (ui_sprite_cfg_loader_load_components_from_cfg(*this, entity, cfg) != 0) {
        APP_CTX_THROW_EXCEPTION(
            entity.world().app(),
            ::std::runtime_error,
            "entity %d(%s) load components from cfg fail!", entity.id(), entity.name());
    }
}

void CfgLoader::addComponentLoader(const char * name, ui_sprite_cfg_load_comp_fun_t fun, void * ctx) {
    if (ui_sprite_cfg_loader_add_comp_loader(*this, name, fun, ctx) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "add component load %s fail!", name);
    }
}

Component & CfgLoader::loadComponent(Entity & entity, const char * name, const char * src_path) {
    Component * r = tryLoadComponent(entity, name, src_path);

    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            entity.world().app(),
            ::std::runtime_error,
            "entity %d(%s) load component %s fail!", entity.id(), entity.name(), name);
    }

    return *r;
}

Component & CfgLoader::loadComponent(Entity & entity, const char * name, cfg_t cfg) {
    Component * r = tryLoadComponent(entity, name, cfg);

    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            entity.world().app(),
            ::std::runtime_error,
            "entity %d(%s) load component %s fail!", entity.id(), entity.name(), name);
    }

    return *r;
}


/*proto fsm*/
Fsm::Fsm & CfgLoader::loadProtoFsm(World & world, const char * name, const char * src_path) {
    Fsm::Fsm * r = tryLoadProtoFsm(world, name, src_path);

    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world.app(),
            ::std::runtime_error,
            "load proto fsm %s from %s fail!", name, src_path);
    }

    return *r;
}

Fsm::Fsm & CfgLoader::loadProtoFsm(World & world, const char * name, cfg_t cfg) {
    Fsm::Fsm * r = tryLoadProtoFsm(world, name, cfg);

    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world.app(),
            ::std::runtime_error,
            "load proto fsm %s from cfg fail!", name);
    }

    return *r;
}

/*instance*/
CfgLoader & CfgLoader::instance(Gd::App::Application & app, const char * name) {
    ui_sprite_cfg_loader_t loader = ui_sprite_cfg_loader_find_nc(app, name);
    if (loader == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "ui_sprite_cfg_loader %s not exist!", name ? name : "default");
    }

    return *(CfgLoader*)loader;
}

}}}



