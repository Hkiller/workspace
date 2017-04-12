#ifndef UI_SPRITE_FSM_TESTENV_WITH_FSM_H
#define UI_SPRITE_FSM_TESTENV_WITH_FSM_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../ui_sprite_fsm_component.h"
#include "../ui_sprite_fsm_ins.h"

namespace ui { namespace sprite { namespace testenv {

class with_fsm : public ::testenv::env<> {
public:
    with_fsm();

    void SetUp();
    void TearDown();

    ui_sprite_fsm_module_t t_s_fsm_repo(void) { return m_fsm_repo; }

    ui_sprite_fsm_action_t t_s_fsm_create_action(const char * action_type);
    ui_sprite_fsm_action_t t_s_fsm_create_action(ui_sprite_entity_t entity, const char * action_type);
    ui_sprite_fsm_action_t t_s_fsm_create_action(ui_sprite_fsm_state_t state, const char * action_type);

    ui_sprite_fsm_state_t t_s_fsm_test_state(ui_sprite_entity_t entity);
    ui_sprite_fsm_state_t t_s_fsm_test_state(void);

    void t_s_fsm_action_update(ui_sprite_fsm_action_t action, float delta);
    void t_s_fsm_action_update(void * action_data, float delta);

    void t_s_fsm_action_enter(ui_sprite_fsm_action_t action);
    void t_s_fsm_action_enter(void * action_data);

    void t_s_fsm_action_endless(ui_sprite_fsm_action_t action);
    void t_s_fsm_action_endless(void * action_data);

private:
    ui_sprite_fsm_module_t m_fsm_repo;
};

}}}

#endif
