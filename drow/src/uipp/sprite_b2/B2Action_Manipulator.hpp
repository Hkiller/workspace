#ifndef UIPP_SPRITE_B2_ACTION_MANIPULATOR_H
#define UIPP_SPRITE_B2_ACTION_MANIPULATOR_H
#include "cpe/utils/prand.h"
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "protocol/uipp/sprite_b2/ui_sprite_b2_evt.h"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectPartExt;
class B2Action_Manipulator : public Fsm::ActionGen<Cpe::Utils::Noncopyable, B2Action_Manipulator> {
public:
    B2Action_Manipulator(Fsm::Action & action);
    B2Action_Manipulator(Fsm::Action & action, B2Action_Manipulator const & o);
    ~B2Action_Manipulator();

    int enter(void);
    void exit(void);

    static const char * NAME;
    static void install(Fsm::Repository & repo);

private:
    void onStop(UI_SPRITE_EVT_B2_STOP const & evt);
    void onSetToEntity(UI_SPRITE_EVT_B2_SET_TO_ENTITY const & evt);
    void onSetLinearVelocityPair(UI_SPRITE_EVT_B2_SET_LINEAR_VELOCITY_PAIR const & evt);
    void onSetLinearVelocityAngle(UI_SPRITE_EVT_B2_SET_LINEAR_VELOCITY_ANGLE const & evt);
    void onRandLineearVelocityAngle(UI_SPRITE_EVT_B2_RAND_LINEAR_VELOCITY_ANGLE const & evt);
    void onPushByForceAngle(UI_SPRITE_EVT_B2_PUSH_BY_IMPULSE_ANGLE const & evt);
    void onPushByForcePair(UI_SPRITE_EVT_B2_PUSH_BY_IMPULSE_PAIR const & evt);
    void onSetAwake(UI_SPRITE_EVT_B2_SET_AWAKE const & evt);
    double prand(void);

    cpe_prand_ctx_t m_rand_ctx;
};

}}}

#endif
