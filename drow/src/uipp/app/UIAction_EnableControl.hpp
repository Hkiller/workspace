#ifndef UIPP_APP_ACTION_ENABLECONTROL_H
#define UIPP_APP_ACTION_ENABLECONTROL_H
#include "cpepp/utils/ObjRef.hpp"
#include "UIActionGen_WithControl.hpp"

namespace UI { namespace App {

class UIAction_EnableControl : public UIActionGen_WithControl<UIAction_EnableControl> {
public:
    UIAction_EnableControl(Sprite::Fsm::Action & action);
    UIAction_EnableControl(Sprite::Fsm::Action & action, UIAction_EnableControl const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);
};

}}

#endif
