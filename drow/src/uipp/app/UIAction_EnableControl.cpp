#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "UIAction_EnableControl.hpp"

namespace UI { namespace App {

UIAction_EnableControl::UIAction_EnableControl(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
}

UIAction_EnableControl::UIAction_EnableControl(Sprite::Fsm::Action & action, UIAction_EnableControl const & o)
    : ActionBase(action, o)
{
}

int UIAction_EnableControl::enter(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return 0;

	control->SetEnable(true);

    return 0;
}

void UIAction_EnableControl::exit(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return;

	control->SetEnable(false);
}

void UIAction_EnableControl::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_EnableControl>(repo)
        .on_enter(&UIAction_EnableControl::enter)
        .on_exit(&UIAction_EnableControl::exit)
        ;
}

const char * UIAction_EnableControl::NAME = "ui-enable-control";

}}

