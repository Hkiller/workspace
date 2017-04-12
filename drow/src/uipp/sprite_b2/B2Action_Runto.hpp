#ifndef UIPP_SPRITE_B2_ACTION_RUNTO_H
#define UIPP_SPRITE_B2_ACTION_RUNTO_H
#include <list>
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "uipp/sprite_b2/B2System.hpp"
#include "protocol/uipp/sprite_b2/ui_sprite_b2_evt.h"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectExt;
class B2Action_RunTo : public Fsm::ActionGen<Cpe::Utils::Noncopyable, B2Action_RunTo> {
    struct wait_data{
        float waiting_time;
        float waiting_distance;
    };

public:
    B2Action_RunTo(Fsm::Action & action);
    B2Action_RunTo(Fsm::Action & action, B2Action_RunTo const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    void setGroundMasks(uint16_t ground_masks);

    static const char * NAME;
    static void install(Fsm::Repository & repo);

private:
    void onRunToEntity(UI_SPRITE_EVT_B2_RUN_TO_ENTITY const & evt);
    void onFollowEntity(UI_SPRITE_EVT_B2_RUN_FOLLOW_ENTITY const & evt);

    P2D::Pair targetPos(void) const;
    P2D::Pair entityPos(uint32_t entity_id, const char * entity_name) const;

    void updateMovingData(P2D::Pair const & pos, float delta);
    void enqueueLastPos(P2D::Pair const & pos);
    float movingAngleFromPos(void) const;
    float incSpeed(float base_speed, float delta) const;
    float decSpeed(float base_speed, float delta) const;
    float stopDistance(float curent_speed) const;
    bool stopOnArrive(void) const { return m_state_data.target_policy != UI_SPRITE_B2_RUN_TARGET_POLICY_FOLLOW_ENTITY; }

    bool checkIsNextPosGood(
        P2D::Pair const & curPos, P2D::Pair const & nextPos, P2D::Pair const & targetPos) const;

    bool isOnGround(void) const;

    void updateSpeed(float v_in_force, float v_not_in_force, float force_angle);

private:
    /*config data*/
    uint16_t m_group_masks;
    UI_SPRITE_B2_RUN_DATA m_state_data;
};

}}}

#endif
