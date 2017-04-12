#ifndef UIPP_SPRITE_FSM_ACTION_H
#define UIPP_SPRITE_FSM_ACTION_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Fsm {

class Action : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_fsm_action_t () const { return (ui_sprite_fsm_action_t)this; }

    ActionMeta const & meta(void) const { return *(ActionMeta const *)ui_sprite_fsm_action_meta(*this); }

    Entity & entity(void) { return *(Entity*)ui_sprite_fsm_action_to_entity(*this); }
    Entity const & entity(void) const { return *(Entity*)ui_sprite_fsm_action_to_entity(*this); }

    World & world(void) { return *(World*)ui_sprite_fsm_action_to_world(*this); }
    World const & world(void) const { return *(World*)ui_sprite_fsm_action_to_world(*this); }

    State & state(void) { return *(State *)ui_sprite_fsm_action_state(*this); }
    State const & state(void) const { return *(State const *)ui_sprite_fsm_action_state(*this); }

    const char * name(void) const { return ui_sprite_fsm_action_name(*this); }
    ui_sprite_fsm_action_state_t runingState(void) const { return ui_sprite_fsm_action_runing_state(*this); }
    ui_sprite_fsm_action_life_circle_t lifeCircle(void) const { return ui_sprite_fsm_action_life_circle(*this); }

    bool isActive(void) const { return ui_sprite_fsm_action_is_active(*this) ? true : false; }
    bool isUpdate(void) const { return ui_sprite_fsm_action_is_update(*this) ? true : false; }
    void startUpdate(void);
    void stopUpdate(void) { ui_sprite_fsm_action_stop_update(*this); }
    void syncUpdate(bool is_update) { ui_sprite_fsm_action_sync_update(*this, is_update ? 1 : 0); }

    void * data(void) { return ui_sprite_fsm_action_data(*this); }
    void const * data(void) const { return ui_sprite_fsm_action_data(*this); }
    size_t size(void) const { return ui_sprite_fsm_action_data_size(*this); }

    int setFollowTo(const char * follow_to) { return ui_sprite_fsm_action_set_follow_to(*this, follow_to); }
};

}}}

#endif
