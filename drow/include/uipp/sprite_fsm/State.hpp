#ifndef UIPP_SPRITE_FSM_STATE_H
#define UIPP_SPRITE_FSM_STATE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "System.hpp"
#include "Action.hpp"

namespace UI { namespace Sprite { namespace Fsm {

class State : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_fsm_state_t () const { return (ui_sprite_fsm_state_t)this; }

    Fsm & fsm(void) { return *(Fsm *)ui_sprite_fsm_state_fsm(*this); }
    Fsm const & fsm(void) const { return *(Fsm const *)ui_sprite_fsm_state_fsm(*this); }

    uint16_t id(void) const { return ui_sprite_fsm_state_id(*this); }
    const char * name(void) const { return ui_sprite_fsm_state_name(*this); }

    State * returnTo(void) { return (State *) ui_sprite_fsm_state_return_to(*this); }
    State const * returnTo(void) const { return (State const *) ui_sprite_fsm_state_return_to(*this); }

    Action * findAction(const char * name, const char * check_type_name = NULL);
    
    template<typename T>
    T * findAction(const char * name) {
        Action * action = findAction(name, T::NAME);
        return action ? (T*)action->data() : NULL;
    }
    
    Action & createAction(const char * name, const char * type_name);

    template<typename T>
    T & createAction(const char * name) {
        return *(T*)createAction(name, T::NAME).data();
    }
};

}}}

#endif
