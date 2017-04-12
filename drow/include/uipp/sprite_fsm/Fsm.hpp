#ifndef UIPP_SPRITE_FSM_INS_H
#define UIPP_SPRITE_FSM_INS_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/MemBuffer.hpp"
#include "cpepp/dr/Utils.hpp"
#include "uipp/sprite/Component.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Fsm {

class Fsm : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_fsm_ins_t () const { return (ui_sprite_fsm_ins_t)this; }

    Entity & entity(void) { return *(Entity*)ui_sprite_fsm_to_entity(*this); }
    Entity const & entity(void) const { return *(Entity*)ui_sprite_fsm_to_entity(*this); }

    World & world(void) { return *(World*)ui_sprite_fsm_to_world(*this); }
    World const & world(void) const { return *(World*)ui_sprite_fsm_to_world(*this); }

    State * findState(uint16_t id) { return (State *)ui_sprite_fsm_state_find_by_id(*this, id); }
    State const * findState(uint16_t id) const { return (State const *)ui_sprite_fsm_state_find_by_id(*this, id); }

    State & createState(const char * name);
    State * findState(const char * name) { return (State *)ui_sprite_fsm_state_find_by_name(*this, name); }
    State const * findState(const char * name) const { return (State const *)ui_sprite_fsm_state_find_by_name(*this, name); }

    State * defaultState(void) { return (State *)ui_sprite_fsm_default_state(*this); }
    State const * defaultState(void) const { return (State const *)ui_sprite_fsm_default_state(*this); }
    void setDefaultState(const char * name);

    State * defaultCallState(void) { return (State *)ui_sprite_fsm_default_call_state(*this); }
    State const * defaultCallState(void) const { return (State const *)ui_sprite_fsm_default_call_state(*this); }
    void setDefaultCallState(const char * name);

    State * currentState(void) { return (State *)ui_sprite_fsm_current_state(*this); }
    State const * currentState(void) const { return (State const *)ui_sprite_fsm_current_state(*this); }

    bool isInState(const char * path) const { return ui_sprite_fsm_is_in_state(*this, path) ? true : false; }
    bool haveState(const char * path) const { return ui_sprite_fsm_have_state(*this, path) ? true : false; }
    
    void copy(Fsm const & from);

    void visitActions(ActionVisitor & visitor);

    const char * path(Cpe::Utils::MemBuffer & buff) const;
};

class ComponentFsm : public Fsm {
public:
    Component & component(void) {
        return *(Component *)ui_sprite_component_from_data((ui_sprite_fsm_ins_t)*this);
    }

    Component const & component(void) const {
        return *(Component const*)ui_sprite_component_from_data((ui_sprite_fsm_ins_t)*this);
    }

    static const char * NAME;
};

class ActionFsm : public Fsm {
public:
    operator ui_sprite_fsm_action_fsm_t () const { return (ui_sprite_fsm_action_fsm_t)this; }

    void setData(dr_data_t data);
    
    template<typename T>
    void setData(T const & data) {
        dr_data d;
        d.m_data = (void*)&data;
        d.m_size = Cpe::Dr::MetaTraits<T>::data_size(data);
        d.m_meta = Cpe::Dr::MetaTraits<T>::META;
        setData(&d);
    }
    
    static const char * NAME;
};

}}}

#endif
