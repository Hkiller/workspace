#include "cpe/utils/string_utils.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/tests-env/with_world.hpp"
#include "../ui_sprite_component_i.h"
#include "../ui_sprite_component_meta_i.h"

namespace ui { namespace sprite { namespace testenv {

static int component_build_cmp(const void * l, const void * r) {
    return strcmp(((with_world::component_builder*)l)->m_name, ((with_world::component_builder*)r)->m_name);
}

with_world::with_world()
    : m_repo(NULL)
    , m_world(NULL)
{
    CPE_SORTED_VECTOR_INIT_FROM_ARRAY(&m_component_builders, m_component_builder_buf, component_build_cmp);
}

void with_world::SetUp() {
    Base::SetUp();

    m_repo = ui_sprite_repository_create(
        envOf<gd::app::testenv::with_app>().t_app(), t_allocrator(),
        NULL,
        envOf<utils::testenv::with_em>().t_em());
    EXPECT_TRUE(m_repo != NULL);
    if (m_repo == NULL) return;

    m_world = ui_sprite_world_create(m_repo, "");
    EXPECT_TRUE(m_world != NULL);

    if (m_world == NULL) {
        ui_sprite_repository_free(m_repo);
        m_repo = NULL;
        return;
    }

    EXPECT_EQ(0, ui_sprite_world_start_tick(m_world));
}

void with_world::TearDown() {
    ui_sprite_world_free(m_world);
    m_world = NULL;

    ui_sprite_repository_free(m_repo);
    m_repo = NULL;

    Base::TearDown();
}

ui_sprite_world_t
with_world::t_s_world() {
    return m_world;
}

void with_world::t_s_entity_enter(ui_sprite_entity_t entity) {
    ASSERT_EQ(0, ui_sprite_entity_enter(entity))
        << "entity " << ui_sprite_entity_id(entity)
        << "(" << ui_sprite_entity_name(entity) << ") enter fail!";
}

void with_world::t_s_component_enter(ui_sprite_component_t component) {
    ASSERT_EQ(0, ui_sprite_component_enter(component))
        << "component " << ui_sprite_component_name(component) << " enter fail";
}

void with_world::t_s_component_enter(void * componen_data) {
    t_s_component_enter(ui_sprite_component_from_data(componen_data));
}

bool with_world::t_s_component_is_active(ui_sprite_component_t componen) {
    return ui_sprite_component_is_active(componen) ? true : false;
}

bool with_world::t_s_component_is_active(void * componen_data) {
    return ui_sprite_component_is_active(ui_sprite_component_from_data(componen_data)) ? true : false;
}

ui_sprite_entity_t
with_world::t_s_entity_create(const char * name, uint8_t auto_active) {
    ui_sprite_entity_t entity = ui_sprite_entity_create(m_world, name, NULL);
    EXPECT_TRUE(entity != NULL) << "create entity " << name << " fail";
    if (entity == NULL) return NULL;

    if (auto_active) t_s_entity_enter(entity);

    return entity;
}

ui_sprite_entity_t
with_world::t_s_entity_create(const char * name, const char * defs, uint8_t auto_active) {
    ui_sprite_entity_t entity = ui_sprite_entity_create(m_world, name, NULL);
    EXPECT_TRUE(entity != NULL) << "create entity " << name << " fail";
    if (entity == NULL) return NULL;

    t_s_entity_setup(entity, defs);

    if (auto_active) t_s_entity_enter(entity);

    return entity;
}

void with_world::t_s_entity_bulk_create(const char * i_entities, uint8_t auto_active) {
    char * entities = t_tmp_strdup(i_entities);

    while(char * p = strchr(entities, ',')) {
        *p = 0;
        t_s_entity_create(entities, auto_active);
        entities = p;
    }

    if (entities[0] != 0) {
        t_s_entity_create(entities, auto_active);
    }
}

ui_sprite_group_t
with_world::t_s_group_create(const char * name, const char * child_entities, const char * child_groups) {
    ui_sprite_group_t group = ui_sprite_group_create(m_world, name);

    EXPECT_TRUE(group != NULL) << "create group " << name << " fail";
    if (group == NULL) return NULL;

    if (child_entities) t_s_group_add_entities(group, child_entities);
    if (child_groups) t_s_group_add_groups(group, child_groups);

    return group;
}

void with_world::t_s_group_add_entities(const char * name, const char * child_entities) {
    ui_sprite_group_t group = ui_sprite_group_find_by_name(m_world, name);
    EXPECT_TRUE(group != NULL) << "group " << name << " not exist";
    if (group == NULL) return;
    t_s_group_add_entities(group, child_entities);
}

void with_world::t_s_group_add_entities(ui_sprite_group_t group, const char * i_child_entities) {
    char * child_entities = t_tmp_strdup(i_child_entities);

    while(char * p = strchr(child_entities, ',')) {
        *p = 0;
        ui_sprite_entity_t entity = ui_sprite_entity_find_by_name(m_world, child_entities);
        EXPECT_TRUE(entity != NULL)
            << "group " << ui_sprite_group_name(group) << " add entity " << child_entities
            << ": entity not exist";
        if (entity) ui_sprite_group_add_entity(group, entity);
        child_entities = p;
    }

    if (child_entities[0] != 0) {
        ui_sprite_entity_t entity = ui_sprite_entity_find_by_name(m_world, child_entities);
        EXPECT_TRUE(entity != NULL)
            << "group " << ui_sprite_group_name(group) << " add entity " << child_entities
            << ": entity not exist";
        if (entity) ui_sprite_group_add_entity(group, entity);
    }
}

void with_world::t_s_group_add_groups(const char * name, const char * child_groups) {
    ui_sprite_group_t group = ui_sprite_group_find_by_name(m_world, name);
    EXPECT_TRUE(group != NULL) << "group " << name << " not exist";
    if (group == NULL) return;
    t_s_group_add_groups(group, child_groups);
}

void with_world::t_s_group_add_groups(ui_sprite_group_t group, const char * i_child_groups) {
    char * child_groups = t_tmp_strdup(i_child_groups);

    while(char * p = strchr(child_groups, ',')) {
        *p = 0;
        ui_sprite_group_t child_group = ui_sprite_group_find_by_name(m_world, child_groups);
        EXPECT_TRUE(child_group != NULL)
            << "group " << ui_sprite_group_name(group) << " add group " << child_groups
            << ": group not exist";
        if (child_group) ui_sprite_group_add_group(group, child_group);
        child_groups = p;
    }

    if (child_groups[0] != 0) {
        ui_sprite_group_t child_group = ui_sprite_group_find_by_name(m_world, child_groups);
        EXPECT_TRUE(child_group != NULL)
            << "group " << ui_sprite_group_name(group) << " add group " << child_groups
            << ": group not exist";
        if (child_group) ui_sprite_group_add_group(group, child_group);
    }
}

void with_world::t_s_component_builder_add(const char * component_name, void * ctx, component_build_fun_t fun) {
    component_builder builder;
    cpe_str_dup(builder.m_name, sizeof(builder.m_name), component_name);
    builder.m_ctx = ctx;
    builder.m_fun = fun;

    EXPECT_EQ(0, cpe_sorted_vector_insert_unique(&m_component_builders, &builder))
        << "component " << component_name << " already have builder!";
}

void with_world::t_s_component_builder_remove(const char * component_name) {
    component_builder key;
    cpe_str_dup(key.m_name, sizeof(key.m_name), component_name);
    void * builder = cpe_sorted_vector_find_first(&m_component_builders, (void*)&key);
    ASSERT_TRUE(builder != NULL) << "component " << component_name << " builder not exist!";
    cpe_sorted_vector_erase(&m_component_builders, builder);
}

void with_world::t_s_entity_setup(ui_sprite_entity_t entity, const char * defs) {
    t_s_entity_setup(entity, envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(defs));
}

void with_world::t_s_entity_setup(ui_sprite_entity_t entity, cfg_t cfg) {
    cfg_it child_it;

    cfg_it_init(&child_it, cfg);

    while(cfg_t child_cfg = cfg_it_next(&child_it)) {
        const char * name = cfg_name(child_cfg);

        if (strcmp(name, "debug") == 0) {
            ui_sprite_entity_set_debug(entity, cfg_as_uint8(child_cfg, ui_sprite_entity_debug(entity)));
        }
        else {
            ui_sprite_component_t component = ui_sprite_component_create(entity, name);
            ASSERT_TRUE(component != NULL) << "entity create component " << name << " fail!";

            component_builder key;
            cpe_str_dup(key.m_name, sizeof(key.m_name), name);
            component_builder * builder = (component_builder *)cpe_sorted_vector_find_first(&m_component_builders, &key);
            ASSERT_TRUE(builder != NULL) << "component " << name << " no builder";

            builder->m_fun(builder->m_ctx, ui_sprite_component_data(component), child_cfg);
        }
    }
}

}}}
