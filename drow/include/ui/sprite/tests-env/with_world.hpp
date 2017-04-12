#ifndef UI_SPRITE_TESTENV_WITH_WORLD_H
#define UI_SPRITE_TESTENV_WITH_WORLD_H
#include "cpe/utils/sorted_vector.h"
#include "cpe/utils/tests-env/test-env.hpp"
#include "../ui_sprite_world.h"

namespace ui { namespace sprite { namespace testenv {

class with_world : public ::testenv::env<> {
public:
    with_world();

    void SetUp();
    void TearDown();

    ui_sprite_repository_t t_s_repo(void) { return m_repo; }
    ui_sprite_world_t t_s_world(void);

    /*entity operations*/
    void t_s_entity_enter(ui_sprite_entity_t entity);
    ui_sprite_entity_t t_s_entity_create(const char * name, uint8_t auto_active = 1);
    ui_sprite_entity_t t_s_entity_create(const char * name, const char * defs, uint8_t auto_active = 1);
    void t_s_entity_bulk_create(const char * entities, uint8_t auto_active = 1);
    ui_sprite_entity_t t_s_entity_find(const char * name);
    ui_sprite_entity_t t_s_entity_find(uint32_t id);
    void t_s_entity_setup(ui_sprite_entity_t entity, const char * defs);
    void t_s_entity_setup(ui_sprite_entity_t entity, cfg_t cfg);

    /*group operations*/
    ui_sprite_group_t t_s_group_create(const char * name, const char * child_entities = NULL, const char * child_groups = NULL);
    void t_s_group_add_entities(ui_sprite_group_t group, const char * child_entities);
    void t_s_group_add_entities(const char * name, const char * child_entities);
    void t_s_group_add_groups(ui_sprite_group_t group, const char * child_groups);
    void t_s_group_add_groups(const char * name, const char * child_groups);

    /*component operations*/
    void t_s_component_enter(ui_sprite_component_t componen);
    void t_s_component_enter(void * componen_data);

    bool t_s_component_is_active(ui_sprite_component_t componen);
    bool t_s_component_is_active(void * componen_data);

    /*component builder*/
    typedef void (*component_build_fun_t)(void * ctx, void * component_data, cfg_t cfg);

    struct component_builder {
        char m_name[64];
        void * m_ctx;
        component_build_fun_t m_fun;
    };

    void t_s_component_builder_add(const char * component_name, void * ctx, component_build_fun_t fun);
    void t_s_component_builder_remove(const char * component_name);

private:
    ui_sprite_repository_t m_repo;
    ui_sprite_world_t m_world;
    component_builder m_component_builder_buf[100];
    cpe_sorted_vector m_component_builders;
};

}}}

#endif
