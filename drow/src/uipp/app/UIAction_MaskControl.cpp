#include "RGUIControl.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_MaskControl.hpp"
#include "EnvExt.hpp"
#include "cpe/pal/pal_strings.h"

namespace UI { namespace App {

UIAction_MaskControl::UIAction_MaskControl(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
	bzero(&m_percent_decorator, sizeof(m_percent_decorator));
}

UIAction_MaskControl::UIAction_MaskControl(Sprite::Fsm::Action & action, UIAction_MaskControl const & o)
    : ActionBase(action, o)
	, m_target_cotrol_name(o.m_target_cotrol_name)
	, m_percent_decorator(o.m_percent_decorator)
	, m_policy(o.m_policy)
    , m_take_time(o.m_take_time)
{
}

void UIAction_MaskControl::setTargetControl(const char * control_name) {
    m_target_cotrol_name = control_name;
    * cpe_str_trim_tail(&m_target_cotrol_name[0] + m_target_cotrol_name.size(), &m_target_cotrol_name[0]) = 0;
}

int UIAction_MaskControl::enter(void) {
    RGUIControl * control = findTargetControl();
    if (control == NULL) return -1;

    RGUIControl * target_control = findTargetControl(m_target_cotrol_name.c_str());
    if (target_control == NULL) return -1;
    
	m_runing_time = 0.0f;
    m_origin_pos = getControlPos(control);
	m_target_pos = getControlPos(target_control);

    //if (m_speed > 0.0f){
	   // m_duration = cpe_math_distance(m_origin_pos.x, m_origin_pos.y, m_target_pos.x, m_target_pos.y) / m_speed;
    //}
    //else if(m_take_time > 0.0f){
    //    m_duration = m_take_time;
    //}
    //else{
    //    APP_CTX_ERROR(
    //        app(), "entity %d(%s): %s: enter: speed=%f,take_tim=%f error",
    //        entity().id(), entity().name(), name(), m_speed, m_take_time);
    //    return -1;
    //}

    control->SetRenderRealX(m_origin_pos.x);
    control->SetRenderRealY(m_origin_pos.y);
	control->SetVisible(true);

	startUpdate();

    return 0;
}

Sprite::P2D::Pair UIAction_MaskControl::getControlPos(RGUIControl * control) {
	Sprite::P2D::Pair control_pos;

	control_pos.x = control->GetRenderRealX() - control->GetRenderRealW() * control->GetPivot().x;
	control_pos.y = control->GetRenderRealY() - control->GetRenderRealH() * control->GetPivot().y;
	return control_pos;
}

void UIAction_MaskControl::update(float delta) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) {
        stopUpdate();
        return;
    }

	m_runing_time += delta;

	float percent = m_runing_time >= m_duration ? 1.0f : m_runing_time / m_duration;

	percent = ui_percent_decorator_decorate(&m_percent_decorator, percent);

	RVector2 putPos(
        m_origin_pos.x + (m_target_pos.x - m_origin_pos.x) * percent,
        m_origin_pos.y + (m_target_pos.y - m_origin_pos.y) * percent);
	control->SetRenderRealPT(putPos);

	if (m_runing_time >= m_duration) {
		stopUpdate();
	}
}

void UIAction_MaskControl::exit(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return;
	control->SetRenderRealPT(RVector2(m_origin_pos.x, m_origin_pos.y));
}

void UIAction_MaskControl::setDecotator(const char* def){
	if(ui_percent_decorator_setup(&m_percent_decorator,def, app().em()) != 0){
		APP_CTX_THROW_EXCEPTION(
			app(), ::std::runtime_error,
			"entity %d(%s): %s: set decorate %s fail",
			entity().id(), entity().name(), name(), def);
	}
}

void UIAction_MaskControl::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_MaskControl>(repo)
        .on_enter(&UIAction_MaskControl::enter)
        .on_exit(&UIAction_MaskControl::exit)
        .on_update(&UIAction_MaskControl::update)
        ;
}

const char * UIAction_MaskControl::NAME = "ui-mask-control";

}}

