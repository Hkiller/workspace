#include <stdio.h>
#include <vector>
#include "RGUIControl.h"
#include "RGUIProgressBar.h"
#include "RGUIPictureCondition.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data.h"
#include "cpepp/utils/MemBuffer.hpp"
#include "gdpp/app/Log.hpp"
#include "gd/app/app_log.h"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_BindValue.hpp"

namespace UI { namespace App {

UIAction_BindValue::UIAction_BindValue(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
}

UIAction_BindValue::UIAction_BindValue(Sprite::Fsm::Action & action, UIAction_BindValue const & o)
    : ActionBase(action, o)
	, m_values(o.m_values)
{
}

UIAction_BindValue::~UIAction_BindValue() {
}

int UIAction_BindValue::enter(void) {
    if (m_values.empty()) {
        APP_CTX_ERROR(app(), "entity %d(%s): %s: no binding value!", entity().id(), entity().name(), NAME);
        return -1;
    }
    
    ::std::vector<const char *> defs;
    for(::std::list<Value_Data>::iterator it = m_values.begin(); it != m_values.end(); ++it) {
        defs.push_back(it->value.c_str());
	}
    
    addAttrMonitorByDefs<1>(&defs[0], (uint16_t)defs.size(), &UIAction_BindValue::onValueUpdated);

    updateAttrs();

    return 0;
}

void UIAction_BindValue::exit(void) {
}

void UIAction_BindValue::onValueUpdated(void) {
    updateAttrs();
}

void UIAction_BindValue::updateAttrs(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return;

    Cpe::Utils::MemBuffer buffer(NULL);
	for(::std::list<Value_Data>::iterator it = m_values.begin(); it != m_values.end(); ++it) {
        const char * value = calcString(it->value.c_str(), buffer);
        if (value == NULL) continue;
        
        plugin_ui_control_set_attr_by_str(control->control(), it->name.c_str(), value);
	}
}

void UIAction_BindValue::addValue(const char * name, const char * value) {
    Value_Data value_data;
    value_data.name = name;
    value_data.value = value;
    m_values.push_back(value_data);
}

void UIAction_BindValue::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_BindValue>(repo)
        .on_enter(&UIAction_BindValue::enter)
        .on_exit(&UIAction_BindValue::exit)
        ;
}

const char * UIAction_BindValue::NAME = "ui-bind-value";

}}

