#ifndef UIPP_SPRITE_FSM_ACTION_META_H
#define UIPP_SPRITE_FSM_ACTION_META_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Fsm {

class ActionMeta : public Cpe::Utils::SimulateObject {
public:
    const char * name(void) const { return ui_sprite_fsm_action_meta_name(*this); } 
    operator ui_sprite_fsm_action_meta_t () const { return (ui_sprite_fsm_action_meta_t)this; }
};

}}}

#endif
