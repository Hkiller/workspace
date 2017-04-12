#include "RGUIWindow.h"
#include "RGUIPictureCondition.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "cpepp/utils/MemBuffer.hpp"
#include "cpe/dr/dr_data_value.h"
#include "plugin/ui/plugin_ui_control_attr_meta.h"

RGUIPictureCondition::RGUIPictureCondition() {
	mIndex = -1;
}

RGUIPictureCondition::~RGUIPictureCondition() {
}

void RGUIPictureCondition::DelRenderDataAll( void ) {
    plugin_ui_control_frame_clear_in_layer(control(), plugin_ui_control_frame_layer_back, NULL);
}

void RGUIPictureCondition::Load(ui_data_control_t control) {
    RGUIPicture::Load(control);

    if (type() == ui_control_type_picture_cond) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        mIndex = data.data.picture_cond.index;
        updateFrames();
    }
}

void RGUIPictureCondition::SetIndex(uint32_t index) {
	if (mIndex == index) return;
    mIndex = index;
    updateFrames();
}

void RGUIPictureCondition::updateFrames(void) {
    uint32_t pos = 0;
    plugin_ui_control_frame_it frame_it;
    plugin_ui_control_frames_by_layer_and_usage(control(), &frame_it, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal);
    while(plugin_ui_control_frame_t frame = plugin_ui_control_frame_it_next(&frame_it)) {
        plugin_ui_control_frame_set_visible(frame, pos == mIndex);
        ++pos;
    }
}

int RGUIPictureCondition::index_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);

    uint32_t v;
    if (dr_value_try_read_uint32(&v, value, NULL) != 0) return -1;

    cast<RGUIPictureCondition>(control)->SetIndex(v);
    return 0;
}

void RGUIPictureCondition::setup(plugin_ui_control_meta_t meta) {
    RGUIPicture::setup(meta);

    plugin_ui_control_attr_meta_create(meta, "index", index_setter, NULL);
}
