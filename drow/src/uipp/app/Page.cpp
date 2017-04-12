#include <map>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "plugin/ui/plugin_ui_state_node.h"
#include "RGUILabel.h"
#include "RGUIPictureCondition.h"
#include "RGUILabelCondition.h"
#include "RGUIProgressBar.h"
#include "RGUIEditBox.h"
#include "RGUIToggle.h"
#include "cpepp/cfg/Node.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Repository.hpp"
#include "uipp/app/Page.hpp"
#include "EnvExt.hpp"
#include "UICenterExt.hpp"

namespace UI { namespace App {

Page::Page() {
}

Page::~Page() {
}

void Page::setLabelText(RGUIControl * control, const char * name, const char * text) {
    if (RGUILabel * c = dynamic_cast<RGUILabel*>(findChild(control, name))) {
        c->SetTextA(text);
    }
}

void Page::setLabelTextColor(RGUIControl * control, const char * name, ui_color const & color) {
    if (RGUILabel * c = dynamic_cast<RGUILabel*>(findChild(control, name))) {
        c->checkUpdatCache();
        c->SetColor(color);
    }
}

void Page::setLabelTextColor(RGUIControl * control, const char * name, const char * color) {
    if (RGUILabel * c = dynamic_cast<RGUILabel*>(findChild(control, name))) {
        c->SetColor(color);
    }
}

void Page::setLabelTextSize(RGUIControl * control, const char * name, uint8_t font_size) {
    if (RGUILabel * c = dynamic_cast<RGUILabel*>(findChild(control, name))) {
        c->SetFontSize(font_size);
    }
}

void Page::setIndex(RGUIControl * control, const char * name, int index) {
    if (RGUIPictureCondition* pic = dynamic_cast<RGUIPictureCondition*>(findChild(control, name))) {
        pic->SetIndex((uint32_t)index);
    }
	else if (RGUILabelCondition* lab = dynamic_cast<RGUILabelCondition*>(findChild(control, name))) {
		lab->SetIndex((uint32_t)index);
	}
}

void Page::setProgressMode(RGUIControl * control, const char * name, uint8_t mode) {
	if (RGUIProgressBar* progress = dynamic_cast<RGUIProgressBar*>(findChild(control, name))) {
		progress->SetMode(mode);
	}
}

bool Page::isProgressComplete(RGUIControl * control, const char * name) {
	if (RGUIProgressBar* progress = dynamic_cast<RGUIProgressBar*>(findChild(control, name))) {
		return progress->GetLastProgress() == progress->GetProgress();
	}
	return false;
}

void Page::setProgress(RGUIControl * control, const char * name, float percent, float speed) {
	if (RGUIProgressBar* progress = dynamic_cast<RGUIProgressBar*>(findChild(control, name))) {
		if(speed > 0.0f)
			progress->SetSpeed(speed);
        
        progress->SetProgress(percent);
    }
}

void Page::setToProgress(RGUIControl * control, const char * name, float percent) {
    if (RGUIProgressBar* progress = dynamic_cast<RGUIProgressBar*>(findChild(control, name))) {
        progress->SetToProgress(percent);
    }
}

plugin_ui_control_frame_t
Page::setBackFrame(RGUIControl * control, const char * name, const char * resource) {
    if (RGUIControl * c = findChild(control, name)) {
        return c->SetBackFrame(resource);
    }
    else {
        return NULL;
    }
}

plugin_ui_control_frame_t
Page::setFloatFrame(RGUIControl * control, const char * name, const char * resource) {
	if (RGUIControl * c = findChild(control, name)) {
		return c->SetFloatFrame(resource);
	}
	else {
		return NULL;
	}
}

plugin_ui_control_frame_t
Page::setDownFrame(RGUIControl * control, const char * name, const char * resource) {
    if (RGUIButton * c = dynamic_cast<RGUIButton*>(findChild(control, name))) {
        return c->SetDownFrame(resource);
    }
    else {
        return NULL;
    }
}

void Page::setBackAndDownFrame(RGUIControl * control, const char * name, const char * resource) {
    if (RGUIButton * c = dynamic_cast<RGUIButton*>(findChild(control, name))) {
        c->SetBackAndDownFrame(resource);
    }
}

bool Page::isChildOf(RGUIControl* control, const char * name) const{
	for(RGUIControl* p = control->GetParent(); p; p = p->GetParent()) {
		if (strcmp(p->name(), name) == 0) return true;
	}
	return false;
}

bool Page::isControlNameWith(RGUIControl* control, const char * str) const{
	return strstr(control->name(), str) != NULL;
}

void Page::setControlEnable(RGUIControl * control, const char * name, bool is_enable) {
	if (RGUIControl * c = findChild(control, name)) {
        c->SetEnable(is_enable);
    }
}

void Page::setControlPushed(RGUIControl * control, const char * name, bool is_paushed) {
	if (RGUIToggle * c = dynamic_cast<RGUIToggle*>(findChild(control, name))) {
        c->SetPushed(is_paushed);
    }
}

void Page::setControlVisible(RGUIControl * control, const char * name, bool is_visible) {
    if (RGUIControl * c = findChild(control, name)) {
        c->SetVisible(is_visible);
    }
}

void Page::setControlColor(RGUIControl * control, const char * name, ui_color const & color) {
    if (RGUIControl * c = findChild(control, name)) {
        c->SetColor(color);
    }
}

void Page::setAcceptClick(RGUIControl * control, const char * name, bool accept_click) {
    if (RGUIControl * c = findChild(control, name)) {
        plugin_ui_control_set_accept_click(c->control(), accept_click ? 1 : 0);
    }
}

void Page::setListCount(RGUIControl * control, const char * name, uint32_t item_count) {
}

::std::string Page::getText(RGUIControl * control, const char * name) {
    if (RGUIEditBox * ebox = dynamic_cast<RGUIEditBox*>(findChild(control, name))) {
        return ebox->GetTextA();
    }
    else {
        return "";
    }
}

void Page::setUserData(RGUIControl * control, const char * data) {
    control->SetUserText(data);
}

void Page::setUserData(RGUIControl * control, const char * name, const char * data) {
    if (RGUIControl * c = findChild(control, name)) {
        setUserData(c, data);
    }
}

const char * Page::curPhaseName(void) {
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(ui_env());
    assert(cur_phase);
    return plugin_ui_phase_node_name(cur_phase);
}

void Page::phaseSwitch(const char * phase_name, const char * load_phase_name, dr_data_t data) {
    plugin_ui_env_phase_switch_by_name(ui_env(),  phase_name, load_phase_name, data);
}

void Page::phaseReset(void) {
    plugin_ui_env_phase_reset(ui_env());
}

void Page::phaseCall(const char * phase_name, const char * load_phase_name, const char * back_phase_name, dr_data_t data) {
    plugin_ui_env_phase_call_by_name(ui_env(),  phase_name, load_phase_name, back_phase_name, data);
}

void Page::phaseBack(void) {
    plugin_ui_env_phase_back(ui_env());
}

/*state operations*/
const char * Page::curStateName(void) {
    plugin_ui_env_t env = ui_env();
    assert(env);
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(env);
    assert(cur_phase);

    plugin_ui_state_node_t cur_state = plugin_ui_state_node_current(cur_phase);
    return cur_state ? plugin_ui_state_node_name(cur_state) : "";
}

void Page::stateCall(
    const char * state_name, bool suspend_old, const char * enter, const char * leave, dr_data_t data,
    plugin_ui_renter_policy_t renter_policy)
{
    plugin_ui_env_t env = ui_env();
    assert(env);
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(env);
    assert(cur_phase);

    plugin_ui_state_node_t cur_state = plugin_ui_state_node_current(cur_phase);
    assert(cur_state);

    plugin_ui_state_node_call(cur_state, state_name, enter, leave, renter_policy, suspend_old, data);
}

void Page::stateSwitch(
    const char * state_name, const char * enter, const char * leave, dr_data_t data)
{
    plugin_ui_env_t env = ui_env();
    assert(env);
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(env);
    assert(cur_phase);
    
    plugin_ui_state_node_t cur_state = plugin_ui_state_node_current(cur_phase);
    assert(cur_state);

    plugin_ui_state_node_switch(cur_state, state_name, enter, leave, data);
}

void Page::stateReset(void) {
    plugin_ui_env_t env = ui_env();
    assert(env);
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(env);
    assert(cur_phase);

    plugin_ui_state_node_reset(cur_phase);
}

void Page::stateBack(void) {
    plugin_ui_env_t env = ui_env();
    assert(env);
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(env);
    assert(cur_phase);

    plugin_ui_state_node_t cur_state = plugin_ui_state_node_current(cur_phase);
    
    plugin_ui_state_node_back(cur_state);
}

void Page::stateBackTo(const char * state) {
    plugin_ui_env_t env = ui_env();
    assert(env);
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(env);
    assert(cur_phase);

    plugin_ui_state_node_t cur_state = plugin_ui_state_node_find_by_process(cur_phase, state);
    if (cur_state == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "stateBackTo: state %s not in stack!", state);
    }
    
    plugin_ui_state_node_back(cur_state);
}

}}
