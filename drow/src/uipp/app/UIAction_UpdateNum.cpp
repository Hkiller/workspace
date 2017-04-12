#include "RGUILabel.h"
#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_UpdateNum.hpp"

namespace UI { namespace App {

UIAction_UpdateNum::UIAction_UpdateNum(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
	bzero(&m_percent_decorator, sizeof(m_percent_decorator));
}

UIAction_UpdateNum::UIAction_UpdateNum(Sprite::Fsm::Action & action, UIAction_UpdateNum const & o)
    : ActionBase(action, o)
	, m_percent_decorator(o.m_percent_decorator)
	, m_take_time(o.m_take_time)
	, m_binding_value(o.m_binding_value)
    , m_init_value(o.m_init_value)
{
}

int UIAction_UpdateNum::enter(void) {
    m_target_num = m_origin_num = 0;

    if (m_binding_value.empty() && m_init_value.empty()) {
		APP_CTX_ERROR(
			app(), "entity %d(%s) ui-update-num: no binding value or init value!",
            entity().id(), entity().name());
		return -1;
    }

    if (!m_binding_value.empty()) {
        addAttrMonitorByDef<1>(m_binding_value.c_str(), &UIAction_UpdateNum::onBindingValueChanged);
        if (m_init_value.empty()) {
            onBindingValueChanged();
        }
    }

    if (!m_init_value.empty()) {
        onInitValue();
    }

    return 0;
}

void UIAction_UpdateNum::update(float delta) {
	m_runing_time += delta;

	float percent = m_runing_time >= m_take_time ? 1.0f : m_runing_time / m_take_time;
	percent = ui_percent_decorator_decorate(&m_percent_decorator, percent);

	int num = m_origin_num + (m_target_num - m_origin_num) * percent;
    setValue(num);

	if (m_runing_time >= m_take_time || m_target_num == m_origin_num) {
        m_target_num = m_origin_num = 0;
		stopUpdate();
	}
}

void UIAction_UpdateNum::exit(void) {
    if (m_target_num != m_origin_num) {
        setValue(m_target_num);
    }
}

void UIAction_UpdateNum::setDecotator(const char* def){
	if(ui_percent_decorator_setup(&m_percent_decorator,def, app().em()) != 0){
		APP_CTX_THROW_EXCEPTION(
			app(), ::std::runtime_error,
			"entity %d(%s): %s: set decorate %s fail",
			entity().id(), entity().name(), name(), def);
	}
}

void UIAction_UpdateNum::startUpdate(int32_t target_num) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return;

	RGUILabel * label = dynamic_cast<RGUILabel*>(control);
	if (label == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s) ui-update-num: control %s is not label!",
			entity().id(), entity().name(), controlName());
		return;
	}

    m_target_num = target_num;
	m_runing_time = 0.0f;
	m_origin_num = atoi(label->GetTextA());

    syncUpdate(true);
}

void UIAction_UpdateNum::setValue(int32_t value) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return;

	RGUILabel * label = dynamic_cast<RGUILabel*>(control);
	if (label == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s) ui-update-num: control %s is not label!",
			entity().id(), entity().name(), controlName());
		return;
	}

    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    label->SetTextA(buf);
}

void UIAction_UpdateNum::onBindingValueChanged(void) {
    int32_t new_value;
	if (ui_sprite_fsm_action_try_calc_int32(
            &new_value, m_binding_value.c_str(), action(), NULL, app().em())
		!= 0)
	{
		APP_CTX_ERROR(
			app(), "entity %d(%s) ui-update-num: calc value from '%s' fail!",
			entity().id(), entity().name(), m_binding_value.c_str());
		return;
	}

    startUpdate(new_value);
}

void UIAction_UpdateNum::onInitValue(void) {
    int32_t new_value;
	if (ui_sprite_fsm_action_try_calc_int32(
            &new_value, m_init_value.c_str(), action(), NULL, app().em())
		!= 0)
	{
		APP_CTX_ERROR(
			app(), "entity %d(%s) ui-update-num: calc value from '%s' fail!",
			entity().id(), entity().name(), m_init_value.c_str());
		return;
	}

    startUpdate(new_value);
}

void UIAction_UpdateNum::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_UpdateNum>(repo)
        .on_enter(&UIAction_UpdateNum::enter)
        .on_exit(&UIAction_UpdateNum::exit)
        .on_update(&UIAction_UpdateNum::update)
        ;
}

const char * UIAction_UpdateNum::NAME = "ui-update-num";

}}

