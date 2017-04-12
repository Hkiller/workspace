#include "cpe/utils/stream_mem.h"
#include "cpe/utils/error.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "ui/sprite/tests-env/with_world.hpp"
#include "ui/sprite/tests-env/with_entity.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_fsm/tests-env/with_fsm.hpp"
#include "../ui_sprite_fsm_ins_action_i.h"
#include "../ui_sprite_fsm_action_meta_i.h"
#include "../ui_sprite_fsm_component_i.h"

namespace ui { namespace sprite { namespace testenv {

with_fsm::with_fsm() : m_fsm_repo(NULL) {
}

void with_fsm::SetUp() {
    Base::SetUp();

    m_fsm_repo = 
        ui_sprite_fsm_module_create(
            envOf<gd::app::testenv::with_app>().t_app(),
            envOf<ui::sprite::testenv::with_world>().t_s_repo(),
            t_allocrator(), NULL,
            envOf<utils::testenv::with_em>().t_em());

    ASSERT_TRUE(m_fsm_repo != NULL);
}

void with_fsm::TearDown() {
    ui_sprite_fsm_module_free(m_fsm_repo);
    m_fsm_repo = NULL;

    Base::TearDown();
}

ui_sprite_fsm_action_t
with_fsm::t_s_fsm_create_action(ui_sprite_entity_t entity, const char * action_type) {
    ui_sprite_fsm_state_t test_state = t_s_fsm_test_state(entity);
    if (test_state == NULL) return NULL;

    return t_s_fsm_create_action(test_state, action_type);
}

ui_sprite_fsm_action_t
with_fsm::t_s_fsm_create_action(ui_sprite_fsm_state_t state, const char * action_type) {
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_create(state, "", action_type);
    EXPECT_TRUE(action != NULL);
    return action;
}

ui_sprite_fsm_action_t
with_fsm::t_s_fsm_create_action(const char * action_type) {
    return t_s_fsm_create_action(envOf<with_entity>().t_s_entity(), action_type);
}

ui_sprite_fsm_state_t
with_fsm::t_s_fsm_test_state(ui_sprite_entity_t entity) {
    ui_sprite_fsm_component_fsm_t fsm_ins = ui_sprite_fsm_component_find(entity);
    if (fsm_ins == NULL) {
        fsm_ins = ui_sprite_fsm_component_create(entity);
        EXPECT_TRUE(fsm_ins != NULL);
        if (fsm_ins == NULL) return NULL;

        ui_sprite_fsm_component_fsm_set_auto_destory(fsm_ins, 0);
        envOf<with_world>().t_s_component_enter(fsm_ins);
    }

    ui_sprite_fsm_state_t test_state = ui_sprite_fsm_state_find_by_name(&fsm_ins->m_ins, "test-state");
    if (test_state == NULL) {
        test_state =ui_sprite_fsm_state_create(&fsm_ins->m_ins, "test-state");
        EXPECT_TRUE(test_state != NULL);
        EXPECT_EQ(0, ui_sprite_fsm_component_fsm_set_state(fsm_ins, "test-state", NULL));
    }

    return test_state;
}

ui_sprite_fsm_state_t
with_fsm::t_s_fsm_test_state(void) {
    return t_s_fsm_test_state(envOf<with_entity>().t_s_entity());
}

void with_fsm::t_s_fsm_action_update(ui_sprite_fsm_action_t action, float delta) {
    ASSERT_TRUE(action->m_is_update)
        << "action " << ui_sprite_fsm_action_name(action) << "(type="
        << ui_sprite_fsm_action_type_name(action) << ") is not in update";

    ASSERT_TRUE(action->m_meta->m_update_fun)
        << "action " << ui_sprite_fsm_action_name(action) << "(type="
        << ui_sprite_fsm_action_type_name(action) << ") not support update";

    action->m_meta->m_update_fun(action, action->m_meta->m_update_fun_ctx, delta);
}

void with_fsm::t_s_fsm_action_update(void * action_data, float delta) {
    t_s_fsm_action_update(ui_sprite_fsm_action_from_data(action_data), delta);
}

void with_fsm::t_s_fsm_action_enter(ui_sprite_fsm_action_t action) {
    ASSERT_EQ(0, ui_sprite_fsm_action_enter(action))
        << "action " << ui_sprite_fsm_action_name(action) << "(type="
        << ui_sprite_fsm_action_type_name(action) << ") enter fail";
}

void with_fsm::t_s_fsm_action_enter(void * action_data) {
    t_s_fsm_action_enter(ui_sprite_fsm_action_from_data(action_data));
}

void with_fsm::t_s_fsm_action_endless(ui_sprite_fsm_action_t action) {
    ui_sprite_fsm_action_set_life_circle(action, ui_sprite_fsm_action_life_circle_endless);
}

void with_fsm::t_s_fsm_action_endless(void * action_data) {
    t_s_fsm_action_endless(ui_sprite_fsm_action_from_data(action_data));
}

}}}
