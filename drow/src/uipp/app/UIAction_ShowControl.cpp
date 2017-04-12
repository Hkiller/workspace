#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "UIAction_ShowControl.hpp"

namespace UI { namespace App {

UIAction_ShowControl::UIAction_ShowControl(Sprite::Fsm::Action & action)
    : ActionBase(action)
	, m_saved_visible_state(false)
{
}

UIAction_ShowControl::UIAction_ShowControl(Sprite::Fsm::Action & action, UIAction_ShowControl const & o)
    : ActionBase(action, o)
	, m_saved_visible_state(o.m_saved_visible_state)
{
}

int UIAction_ShowControl::enter(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return 0;
	m_saved_visible_state = control->WasVisible();
	control->SetVisible(true);

    return 0;
}

void UIAction_ShowControl::exit(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return;

	control->SetVisible(m_saved_visible_state);
}

void UIAction_ShowControl::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_ShowControl>(repo)
        .on_enter(&UIAction_ShowControl::enter)
        .on_exit(&UIAction_ShowControl::exit)
        ;
}

const char * UIAction_ShowControl::NAME = "ui-show-control";

}}
