#ifndef UIPP_APP_ACTION_DISABLECONTROL_H
#define UIPP_APP_ACTION_DISABLECONTROL_H
#include "cpepp/utils/ObjRef.hpp"
#include "UIActionGen_WithControl.hpp"

namespace UI { namespace App {

class UIAction_DisableControl : public UIActionGen_WithControl<UIAction_DisableControl> {
public:
    UIAction_DisableControl(Sprite::Fsm::Action & action);
    UIAction_DisableControl(Sprite::Fsm::Action & action, UIAction_DisableControl const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);
};

}}

#endif
