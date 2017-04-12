#include "RGUIProgressBar.h"
#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_UpdateProgress.hpp"

namespace UI { namespace App {

UIAction_UpdateProgress::UIAction_UpdateProgress(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
}

UIAction_UpdateProgress::UIAction_UpdateProgress(Sprite::Fsm::Action & action, UIAction_UpdateProgress const & o)
    : ActionBase(action, o)
	, m_binding_value(o.m_binding_value)
	, m_speed(o.m_speed)
    , m_take_time(o.m_take_time)
	, m_alphaMove(o.m_alphaMove)
{
}

int UIAction_UpdateProgress::enter(void) {
    addAttrMonitorByDef<1>(m_binding_value.c_str(), &UIAction_UpdateProgress::onValueUpdated);

    startUpdate();
    
    return 0;
}

void UIAction_UpdateProgress::update(float delta) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) {
        stopUpdate();
        return;
    }

	RGUIProgressBar * progressBar = dynamic_cast<RGUIProgressBar*>(control);
	if (progressBar == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s) ui-update-progreass: control %s is not label!",
			entity().id(), entity().name(), controlName());
		return;
	}

	if(progressBar->GetLastProgress() == m_target_progress) {
		stopUpdate();
	}
}

void UIAction_UpdateProgress::exit(void) {
}

void UIAction_UpdateProgress::onValueUpdated(void) {
    startUpdate();
}

void UIAction_UpdateProgress::startUpdate(void) {
    RGUIControl * control = findTargetControl();
	if (control == NULL) return;

	RGUIProgressBar * progressBar = dynamic_cast<RGUIProgressBar*>(control);
	if (progressBar == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s) ui-update-progreass: control %s is not label!",
			entity().id(), entity().name(), controlName());
		return;
	}

	if (ui_sprite_fsm_action_try_calc_float(
		&m_target_progress, m_binding_value.c_str(), action(), NULL, app().em())
		!= 0)
	{
		APP_CTX_ERROR(
			app(), "entity %d(%s) ui-update-progreass): calc value from %s fail!",
			entity().id(), entity().name(), m_binding_value.c_str());
		return;
	}

	setMode(progressBar);
	progressBar->SetAlphaMove(m_alphaMove);
	
	progressBar->SetProgress(m_target_progress);

    if(m_speed > 0.0f)
        progressBar->SetSpeed(m_speed);

    if(m_take_time > 0.0f)
        progressBar->SetMoveDuration(m_take_time);

	syncUpdate(true);
}

void UIAction_UpdateProgress::setMode(RGUIProgressBar * progressBar) {
	uint8_t mode = progressBar->GetMode();

	if(m_target_progress > progressBar->GetProgress()) {
		if(mode == PM_XDEC) {
			mode = PM_XINC;
		}
		else if(mode == PM_XDEC_R) {
			mode = PM_XINC_R;
		}
		else	 if(mode == PM_YDEC) {
			mode = PM_YINC;
		}
		else if(mode == PM_YDEC_R) {
			mode = PM_YINC_R;
		}
	}
	else {
		if(mode == PM_XINC) {
			mode = PM_XDEC;
		}
		else if(mode == PM_XINC_R) {
			mode = PM_XDEC_R;
		}
		else	 if(mode == PM_YINC) {
			mode = PM_YDEC;
		}
		else if(mode == PM_YINC_R) {
			mode = PM_YDEC_R;
		}
	}

	progressBar->SetMode(mode);
}

void UIAction_UpdateProgress::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_UpdateProgress>(repo)
        .on_enter(&UIAction_UpdateProgress::enter)
        .on_exit(&UIAction_UpdateProgress::exit)
        .on_update(&UIAction_UpdateProgress::update)
        ;
}

const char * UIAction_UpdateProgress::NAME = "ui-update-progress";

}}

