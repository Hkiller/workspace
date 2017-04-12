#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "UIAction_HideControl.hpp"

namespace UI { namespace App {

UIAction_HideControl::UIAction_HideControl(Sprite::Fsm::Action & action)
    : ActionBase(action)
	, m_saved_visible_state(false)
{
}

UIAction_HideControl::UIAction_HideControl(Sprite::Fsm::Action & action, UIAction_HideControl const & o)
    : ActionBase(action, o)
	, m_saved_visible_state(o.m_saved_visible_state)
{
}

int UIAction_HideControl::enter(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return 0;
	m_saved_visible_state = control->WasVisible();
	control->SetVisible(false);

    return 0;
}

void UIAction_HideControl::exit(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return;

	control->SetVisible(m_saved_visible_state);
}

void UIAction_HideControl::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_HideControl>(repo)
        .on_enter(&UIAction_HideControl::enter)
        ;
}

const char * UIAction_HideControl::NAME = "ui-hide-control";

}}

