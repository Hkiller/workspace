#include "cpe/dr/dr_data_value.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_control_attr_meta.h"
#include "RGUIWindow.h"
#include "RGUIToggle.h"

RGUIToggle::RGUIToggle() 
    : mProcessing(0)
    , mPushed(0)
    , mGroup((uint32_t)-1)
    , mSelectTop(0)
{
}

RGUIToggle::~RGUIToggle() {
}

void RGUIToggle::SetPushed(uint8_t flag, uint8_t fire_event) {
    flag = flag ? 1 : 0;
    
	if (mPushed == flag) return;

    mPushed = flag;
    if (mProcessing) return;

    if (fire_event) {
        mProcessing = 1;
        plugin_ui_control_dispatch_event(
            control(), control(),
            plugin_ui_event_toggle_click, plugin_ui_event_dispatch_to_self_and_parent);
        mProcessing = 0;
    }

    if (mPushed != flag) return; /*在事件处理过程中已经改回去了 */
    
    plugin_ui_control_set_usage_render(control(), plugin_ui_control_frame_usage_normal,!mPushed);
    plugin_ui_control_set_usage_render(control(), plugin_ui_control_frame_usage_down, mPushed);
    if (mPushed) {
        startDownAnim();
    }
    else {
        startRiseAnim();
    }
    
    if (!mPushed) return;
    
    RGUIControl * parent = GetParent();
	if (parent == NULL) return;
    
    if (mGroup != (uint32_t)-1) {
        plugin_ui_control_it child_it;
        plugin_ui_control_childs(parent->control(), &child_it);
        for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
            child;
            child = plugin_ui_control_it_next(&child_it))
        {
            RGUIToggle* toggle = RGUIControl::cast<RGUIToggle>(child);
            if (toggle && toggle != this && toggle->mPushed && toggle->mGroup == mGroup) {
                toggle->mPushed = 0;
                plugin_ui_control_set_usage_render(toggle->control(), plugin_ui_control_frame_usage_normal, 1);
                plugin_ui_control_set_usage_render(toggle->control(), plugin_ui_control_frame_usage_down, 0);
                toggle->startRiseAnim();
            }
        }
    }
        
    if(mSelectTop) {
        parent->DelChild(this, false);
        parent->AddChild(this, false);
    }
}

void RGUIToggle::Load(ui_data_control_t control) {
    RGUIButton::Load(control);

    if (type() == ui_control_type_toggle) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUILabel::Load(data.data.toggle.text);
        RGUIButton::Load(data.data.toggle.down);
        mGroup = data.data.toggle.group.group;
    }
}

void RGUIToggle::on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIButton::on_mouse_click(ctx, from_control, event);

    if (ctx == from_control) {
        RGUIToggle * c = cast<RGUIToggle>(from_control);
        if (c->mGroup == (uint32_t)-1 || !c->mPushed)
            c->SetPushed(!c->mPushed);
    }
}

int RGUIToggle::pushed_setter(plugin_ui_control_t control, dr_value_t value) {
    uint8_t v;
    if (dr_value_try_read_uint8(&v, value, NULL) != 0) return -1;
    cast<RGUIToggle>(control)->SetPushed(v ? true : false);
    return 0;
}

int RGUIToggle::pushed_getter(plugin_ui_control_t control, dr_value_t value) {
    value->m_type = CPE_DR_TYPE_UINT8;
    value->m_meta = NULL;
    value->m_data = (void*)&cast<RGUIToggle>(control)->mPushed;
    value->m_size = sizeof(uint8_t);
    return 0;
}

void RGUIToggle::setup(plugin_ui_control_meta_t meta) {
    RGUIButton::setup(meta);
    plugin_ui_control_attr_meta_create(meta, "pushed", pushed_setter, pushed_getter);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_click, plugin_ui_event_scope_self, on_mouse_click);    
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_down, plugin_ui_event_scope_self, NULL);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_self, NULL);
}
