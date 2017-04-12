#include <stdio.h>
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
#include "UIAction_SetValue.hpp"

namespace UI { namespace App {

UIAction_SetValue::UIAction_SetValue(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
}

UIAction_SetValue::UIAction_SetValue(Sprite::Fsm::Action & action, UIAction_SetValue const & o)
    : ActionBase(action, o)
	, m_values(o.m_values)
{
}

UIAction_SetValue::~UIAction_SetValue() {
}

int UIAction_SetValue::enter(void) {
	RGUIControl * control = findTargetControl();
	if (control == NULL) return 0;

    int rv = 0;

    Cpe::Utils::MemBuffer buffer(NULL);
	for(::std::list<Value_Data>::iterator it = m_values.begin(); it != m_values.end(); ++it) {
        const char * value = calcString(it->value.c_str(), buffer);
        if (value == NULL) continue;
        
        if (plugin_ui_control_set_attr_by_str(control->control(), it->name.c_str(), value) != 0) {
            rv = -1;
        }
	}

    return rv;
}

void UIAction_SetValue::exit(void) {
}

void UIAction_SetValue::addValue(const char * name, const char * value) {
    Value_Data value_data;
    value_data.name = name;
    value_data.value = value;
    m_values.push_back(value_data);
}

void UIAction_SetValue::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_SetValue>(repo)
        .on_enter(&UIAction_SetValue::enter)
        .on_exit(&UIAction_SetValue::exit)
        ;
}

const char * UIAction_SetValue::NAME = "ui-set-value";

}}

