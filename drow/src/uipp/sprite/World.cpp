#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_world_attr.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Repository.hpp"

namespace UI { namespace Sprite {

Gd::App::Application & World::app(void) {
    return repository().app();
}

Gd::App::Application const & World::app(void) const {
    return repository().app();
}

WorldRes & World::res(const char * name) {
    WorldRes * e = findRes(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s res %s not exist!", this->name(), name);
    }

    return *e;
}

WorldRes const & World::res(const char * name) const {
    WorldRes const * e = findRes(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s res %s not exist!", this->name(), name);
    }

    return *e;
}

WorldRes & World::createRes(const char * name, size_t size) {
    ui_sprite_world_res_t res = ui_sprite_world_res_create(*this, name, size);

    if (res == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s create res %s(size=%d) fail!", this->name(), name, (int)size);
    }

    return *(WorldRes*)res;
}

void World::removeRes(const char * name) {
    if (ui_sprite_world_res_t res = ui_sprite_world_res_find(*this, name)) {
        ui_sprite_world_res_free(res);
    }
}

Entity & World::entity(const char * name) {
    Entity * e = findEntity(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s entity %s not exist!", this->name(), name);
    }

    return *e;
}

Entity const & World::entity(const char * name) const {
    Entity const * e = findEntity(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s entity %s not exist!", this->name(), name);
    }

    return *e;
}

Entity & World::entity(uint32_t id) {
    Entity * e = findEntity(id);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s entity %d not exist!", name(), id);
    }

    return *e;
}

Entity const & World::entity(uint32_t id) const {
    Entity const * e = findEntity(id);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s entity %d not exist!", name(), id);
    }

    return *e;
}

Entity & World::createEntity(const char * name, const char * proto) {
    ui_sprite_entity_t entity = ui_sprite_entity_create(*this, name, proto);

    if (entity == NULL) {
        if (proto) {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "world %s create entity %s(proto=%s) fail!", this->name(), name, proto);
        }
        else {
            APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s create entity %s fail!", this->name(), name);
        }
    }

    return *(Entity*)entity;
}

Entity & World::proto(const char * name) {
    Entity * e = findProto(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s proto %s not exist!", this->name(), name);
    }

    return *e;
}

Entity const & World::proto(const char * name) const {
    Entity const * e = findProto(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s proto %s not exist!", this->name(), name);
    }

    return *e;
}

Entity & World::createProto(const char * name) {
    ui_sprite_entity_t entity = ui_sprite_entity_proto_create(*this, name);

    if (entity == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s create proto %s fail!", this->name(), name);
    }

    return *(Entity*)entity;
}

void World::removeProto(const char * name) {
    ui_sprite_entity_t entity = ui_sprite_entity_proto_find(*this, name);
    if (entity) {
        ui_sprite_entity_free(entity);
    }
}

Group & World::group(const char * name) {
    Group * e = findGroup(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s group %s not exist!", this->name(), name);
    }

    return *e;
}

Group const & World::group(const char * name) const {
    Group const * e = findGroup(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s group %s not exist!", this->name(), name);
    }

    return *e;
}

Group & World::group(uint32_t id) {
    Group * e = findGroup(id);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s group %d not exist!", name(), id);
    }

    return *e;
}

Group const & World::group(uint32_t id) const {
    Group const * e = findGroup(id);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s group %d not exist!", name(), id);
    }

    return *e;
}

Group & World::createGroup(const char * name) {
    ui_sprite_group_t group = ui_sprite_group_create(*this, name);

    if (group == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "world %s create group %s fail!", this->name(), name);
    }

    return *(Group*)group;
}

void World::addUpdator(ui_sprite_world_update_fun_t fun, void * ctx) {
    if (ui_sprite_world_add_updator(*this, fun, ctx) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s add updator fail!", name());
    }
}

void World::setUpdatorPriority(void * ctx, int8_t priority) {
    if (ui_sprite_world_set_updator_priority(*this, ctx, priority) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s set updator priority fail!", name());
    }
}

void World::copyData(ui_sprite_world_t from_world) {
    if (ui_sprite_world_copy_datas(*this, from_world) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s copy data from %s fail!", name(), ui_sprite_world_name(from_world));
    }
}

void World::setData(LPDRMETA meta, void const * data, size_t data_size) {
    dr_data d = { meta, (void*)data, data_size };
    
    if (ui_sprite_world_set_data(*this, &d) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s set data %s(size=%d) fail!", name(), dr_meta_name(meta), (int)data_size);
    }
}

void * World::findData(LPDRMETA meta) {
    dr_data_t data = ui_sprite_world_find_data(*this, dr_meta_name(meta));
    return data && data->m_meta == meta ? data->m_data : NULL;
}

void const * World::findData(LPDRMETA meta) const {
    dr_data_t data = ui_sprite_world_find_data(*this, dr_meta_name(meta));
    return data && data->m_meta == meta ? data->m_data : NULL;
}

void * World::data(LPDRMETA meta) {
    dr_data_t data = ui_sprite_world_find_data(*this, dr_meta_name(meta));
    if (data == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s no data %s!", name(), dr_meta_name(meta));
    }

    if (data->m_meta != meta) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s data %s meta mismatch!", name(), dr_meta_name(meta));
    }

    return data->m_data;
}

void const * World::data(LPDRMETA meta) const {
    dr_data_t data = ui_sprite_world_find_data(*this, dr_meta_name(meta));
    if (data == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s no data %s!", name(), dr_meta_name(meta));
    }

    if (data->m_meta != meta) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s data %s meta mismatch!", name(), dr_meta_name(meta));
    }

    return data->m_data;
}

void * World::data(const char * meta_name) {
    dr_data_t data = ui_sprite_world_find_data(*this, meta_name);
    if (data == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s no data %s!", name(), meta_name);
    }

    return data->m_data;
}

void const * World::data(const char * meta_name) const {
    dr_data_t data = ui_sprite_world_find_data(*this, meta_name);
    if (data == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s no data %s!", name(), meta_name);
    }

    return data->m_data;
}

void World::setAttr(const char * attrName, int8_t v) {
    if (ui_sprite_world_set_attr_int8(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to %d fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, uint8_t v) {
    if (ui_sprite_world_set_attr_uint8(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to %d fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, int16_t v) {
    if (ui_sprite_world_set_attr_int16(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to %d fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, uint16_t v) {
    if (ui_sprite_world_set_attr_uint16(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to %d fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, int32_t v) {
    if (ui_sprite_world_set_attr_uint32(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to %d fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, uint32_t v) {
    if (ui_sprite_world_set_attr_uint32(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to " FMT_UINT32_T " fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, int64_t v) {
    if (ui_sprite_world_set_attr_int64(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to " FMT_INT64_T " fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, uint64_t v) {
    if (ui_sprite_world_set_attr_uint64(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to " FMT_UINT64_T " fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, float v) {
    if (ui_sprite_world_set_attr_float(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to %f fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, double v) {
    if (ui_sprite_world_set_attr_double(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to %f fail!", this->name(), attrName, v);
    }
}

void World::setAttr(const char * attrName, const char * v) {
    if (ui_sprite_world_set_attr_string(*this, attrName, v) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "world %s: set attr %s to %s fail!", this->name(), attrName, v);
    }
}

World & World::create(Repository & repo, const char * name) {
    ui_sprite_world_t world = ui_sprite_world_create(repo, name);

    if (world == NULL) {
        APP_CTX_THROW_EXCEPTION(repo.app(), ::std::runtime_error, "create world fail!");
    }

    return *(World *)world;
}

World & World::instance(Repository & repo, const char * name) {
    ui_sprite_world_t world = ui_sprite_world_find(repo, name);

    if (world == NULL) {
        APP_CTX_THROW_EXCEPTION(repo.app(), ::std::runtime_error, "world %s not exist!", name);
    }

    return *(World *)world;
}

World & World::instance(Gd::App::Application & app, const char * name) {
    return instance(Repository::instance(app, NULL), name);
}

World * World::find_instance(Repository & repo, const char * name) {
    ui_sprite_world_t world = ui_sprite_world_find(repo, name);

    return (World *)world;
}

World * World::find_instance(Gd::App::Application & app, const char * name) {
    return find_instance(Repository::instance(app, NULL), name);
}

::std::auto_ptr<EntityIterator>
World::entities(mem_allocrator_t alloc) {
    ui_sprite_entity_it_t it = ui_sprite_world_entities(alloc, *this);
    return ::std::auto_ptr<EntityIterator>(new EntityIterator(it));
}

}}
