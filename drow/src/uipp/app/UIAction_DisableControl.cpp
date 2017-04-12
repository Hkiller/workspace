#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "UIAction_DisableControl.hpp"

namespace UI { namespace App {

UIAction_DisableControl::UIAction_DisableControl(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
}

UIAction_DisableControl::UIAction_DisableControl(Sprite::Fsm::Action & action, UIAction_DisableControl const & o)
    : ActionBase(action, o)
{
}

int UIAction_DisableControl::enter(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return 0;

	control->SetEnable(false);

    return 0;
}

void UIAction_DisableControl::exit(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return;

	control->SetEnable(true);
}

void UIAction_DisableControl::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_DisableControl>(repo)
        .on_enter(&UIAction_DisableControl::enter)
        .on_exit(&UIAction_DisableControl::exit)
        ;
}

const char * UIAction_DisableControl::NAME = "ui-disable-control";

}}

