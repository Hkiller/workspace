#ifndef UIPP_SPRITE_FSM_ACTION_VISITOR_H
#define UIPP_SPRITE_FSM_ACTION_VISITOR_H
#include "System.hpp"

namespace UI { namespace Sprite { namespace Fsm {

class ActionVisitor {
public:
    virtual void onAction(Action & action) = 0;
    virtual ~ActionVisitor();
};

}}}

#endif
