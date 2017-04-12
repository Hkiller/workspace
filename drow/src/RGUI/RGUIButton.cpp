#include "cpe/utils/math_ex.h"
#include "cpepp/utils/MemBuffer.hpp"
#include "cpe/dr/dr_data_value.h"
#include "gdpp/app/Log.hpp"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_cfg.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_touch_track.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin/ui/plugin_ui_control_attr_meta.h"
#include "RGUIButton.h"
#include "RGUIControlFrameScale.hpp"
#include "RGUIControlFrameMove.hpp"

RGUIButton::RGUIButton(const char * layout)
    : RGUILabel(layout)
    , m_cfg(NULL)
{
    m_down_text = plugin_ui_control_frame_create_by_frame(
        control(),
        plugin_ui_control_frame_layer_text, plugin_ui_control_frame_usage_down,
        m_text_frame);
}

RGUIButton::~RGUIButton() {
    if (m_cfg) {
        mem_free(allocrator(), m_cfg);
        m_cfg = NULL;
    }
}

void RGUIButton::Load(ui_data_control_t control_t) {
    RGUILabel::Load(control_t);

    if (type() == ui_control_type_button) {
        UI_CONTROL const & data = *ui_data_control_data(control_t);
        RGUILabel::Load(data.data.button.text);
        Load(data.data.button.down);
        createDefaultDownFrames();;
    }

#ifdef _SHOW_BUTTON_RANGE
    // SetDrawColor(true);
	// SetAlpha(0.5f);
#endif
}

void RGUIButton::createDefaultDownFrames(void) {
    if (plugin_ui_control_frame_count_by_usage(control(), plugin_ui_control_frame_usage_down) == 0) {
        ui_runtime_render_second_color_t down_color = NULL;

        if (plugin_ui_control_accept_global_down_color(control())) {
            down_color = plugin_ui_env_cfg_down_color(ui_env());
        }

        plugin_ui_control_frame_it frame_it;
        plugin_ui_control_frames_by_usage(control(), &frame_it, plugin_ui_control_frame_usage_normal);
        while(plugin_ui_control_frame_t frame = plugin_ui_control_frame_it_next(&frame_it)) {
            plugin_ui_control_frame_t down_frame =
                plugin_ui_control_frame_create_by_frame(
                    control(), plugin_ui_control_frame_layer(frame), plugin_ui_control_frame_usage_down, frame);
            if (down_color) {
                ui_runtime_render_obj_ref_set_second_color(
                    plugin_ui_control_frame_render_obj_ref(down_frame), down_color);
            }
        }
    }
}

void RGUIButton::Load(UI_CONTROL_DOWN const & data) {
    if (data.down_dist.value[0] != 0.0f || data.down_dist.value[1] != 0.0f || data.down_scale.value[0] != 0.0f || data.down_scale.value[1] != 0.0f) {
        plugin_ui_cfg_button & cfg_button = check_create_button_cfg();
        cfg_button.m_pos_adj.x = data.down_dist.value[0];
        cfg_button.m_pos_adj.y = data.down_dist.value[1];
        cfg_button.m_scale_adj.x = data.down_scale.value[0];
        cfg_button.m_scale_adj.y = data.down_scale.value[1];
        cfg_button.m_down_duration = 0.0f;
        cfg_button.m_raise_duration = 0.0f;
    }

    // if (m_down_text) {
    //     plugin_ui_control_frame_free(m_down_text);
    //     m_down_text = NULL;
    // }

    // m_down_text_id = data.down_text.text_key;
    // if (m_down_text_id || data.down_text.text_id) {
    //     m_down_text = create_text_frame("basic", plugin_ui_control_frame_usage_down);
    //     update_layout_font(m_down_text, data.down_text.font_info, data.down_text.back_drow);
    // }
}

struct plugin_ui_cfg_button const & RGUIButton::button_cfg(void) const {
    return m_cfg ? *m_cfg : * plugin_ui_env_cfg_button(ui_env(), type());
}

struct plugin_ui_cfg_button &
RGUIButton::check_create_button_cfg(void) {
    if (m_cfg == NULL) {
        m_cfg = (plugin_ui_cfg_button_t)mem_alloc(allocrator(), sizeof(struct plugin_ui_cfg_button));
        if (m_cfg == NULL) {
            APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "RGUIButton::check_create_button_cfg: alloc cfg fail");
        }
        *m_cfg = * plugin_ui_env_cfg_button(ui_env(), type());
    }

    return *m_cfg;
}

void RGUIButton::startRiseAnim(void) {
    plugin_ui_aspect_t lock_aspect = this->lock_aspect();
    
    struct plugin_ui_cfg_button const & cfg = button_cfg();
    removeAnimAll(lock_aspect);
    
    if (cfg.m_raise_duration > 0.0f) {
        createAnim<Drow::ControlFrameMove>(lock_aspect)
            .setDecorator(cfg.m_raise_decorator)
            .setTakeTime(cfg.m_raise_duration)
            .setTargetPos(UI_VECTOR_2_ZERO);
    }
    else {
        SetAllFramePos(UI_VECTOR_2_ZERO);
    }

    if (cfg.m_raise_duration > 0.0f) {
        createAnim<Drow::ControlFrameScale>(lock_aspect)
            .setDecorator(cfg.m_raise_decorator)
            .setTakeTime(cfg.m_raise_duration)
            .setTargetScale(UI_VECTOR_2_IDENTITY);
    }
    else {
        SetAllFrameScale(UI_VECTOR_2_IDENTITY);
    }
}

void RGUIButton::startDownAnim(void) {
    plugin_ui_aspect_t lock_aspect = this->lock_aspect();
    
    struct plugin_ui_cfg_button const & cfg = button_cfg();
    removeAnimAll(lock_aspect);
    
    if (cfg.m_pos_adj != UI_VECTOR_2_ZERO) {
        if (cfg.m_down_duration > 0.0f) {
            createAnim<Drow::ControlFrameMove>(lock_aspect)
                .setDecorator(cfg.m_down_decorator)
                .setTakeTime(cfg.m_down_duration)
                .setTargetPos(cfg.m_pos_adj);
        }
        else {
            SetAllFramePos(cfg.m_pos_adj);
        }
    }

    if (cfg.m_scale_adj != UI_VECTOR_2_IDENTITY) {
        if (cfg.m_down_duration > 0.0f) {
            createAnim<Drow::ControlFrameScale>(lock_aspect)
                .setDecorator(cfg.m_down_decorator)
                .setTakeTime(cfg.m_down_duration)
                .setTargetScale(cfg.m_scale_adj);
        }
        else {
            SetAllFrameScale(cfg.m_scale_adj);
        }
    }
}

void RGUIButton::on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    if (plugin_ui_control_enable(from_control)) {
        plugin_ui_control_set_usage_render(from_control, plugin_ui_control_frame_usage_down, 1);    
        plugin_ui_control_set_usage_render(from_control, plugin_ui_control_frame_usage_normal, 0);
        RGUIControl::cast<RGUIButton>(from_control)->startDownAnim();
    }
    RGUILabel::on_mouse_down(ctx, from_control, event);
}

void RGUIButton::on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    plugin_ui_control_set_usage_render(from_control, plugin_ui_control_frame_usage_down, 0);
    plugin_ui_control_set_usage_render(from_control, plugin_ui_control_frame_usage_normal, 1);
    RGUIControl::cast<RGUIButton>(from_control)->startRiseAnim();
    RGUILabel::on_mouse_rise(ctx, from_control, event);
}

bool RGUIButton::SetBackAndDownFrame( ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy, plugin_ui_aspect_t aspect) {
    plugin_ui_control_frame_clear_in_layer(control(), plugin_ui_control_frame_layer_back, aspect);

    removeAnimAll(lock_aspect());
    
    return AddBackAndDownFrame(render_obj_ref, pos_policy, aspect);
}

bool RGUIButton::SetBackAndDownFrame(const char * res, plugin_ui_aspect_t aspect) {
    plugin_ui_control_frame_clear_in_layer(control(), plugin_ui_control_frame_layer_back, aspect);
    return AddBackAndDownFrame(res, aspect);
}

bool RGUIButton::AddBackAndDownFrame( ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy, plugin_ui_aspect_t aspect) {
    plugin_ui_control_frame_t frame_back = plugin_ui_control_frame_create(control(), plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, render_obj_ref);
    if (frame_back == NULL) return false;
    plugin_ui_control_frame_set_base_pos(frame_back, pos_policy);

    ui_runtime_render_obj_ref_t obj_ref = ui_runtime_render_obj_ref_clone(plugin_ui_control_frame_render_obj_ref(frame_back));
    if (obj_ref == NULL) {
        plugin_ui_control_frame_free(frame_back);
        return false;
    }
    
    plugin_ui_control_frame_t frame_down = plugin_ui_control_frame_create(control(), plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, render_obj_ref);
    if (frame_down == NULL) {
        plugin_ui_control_frame_free(frame_back);
        return false;
    }
    plugin_ui_control_frame_set_base_pos(frame_down, pos_policy);

    if (aspect) {
        plugin_ui_aspect_control_frame_add(aspect, frame_back, 1);
        plugin_ui_aspect_control_frame_add(aspect, frame_down, 1);
    }

    return true;
}

bool RGUIButton::AddBackAndDownFrame(const char * res, plugin_ui_aspect_t aspect) {
    if (res[0] == 0) return true;

    plugin_ui_control_frame_t frame_back = plugin_ui_control_frame_create_by_res(control(), plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, res);
    if (frame_back == NULL) return false;

    ui_runtime_render_obj_ref_t obj_ref = ui_runtime_render_obj_ref_clone(plugin_ui_control_frame_render_obj_ref(frame_back));
    if (obj_ref == NULL) {
        plugin_ui_control_frame_free(frame_back);
        return false;
    }
    
    plugin_ui_control_frame_t frame_down = plugin_ui_control_frame_create(control(), plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, obj_ref);
    if (frame_down == NULL) {
        ui_runtime_render_obj_ref_free(obj_ref);
        plugin_ui_control_frame_free(frame_back);
        return false;
    }
    
    if (aspect) {
        plugin_ui_aspect_control_frame_add(aspect, frame_back, 1);
        plugin_ui_aspect_control_frame_add(aspect, frame_down, 1);
    }

    return true;
}

int RGUIButton::add_down_res_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);

    if (str_value == NULL) return -1;
    if (plugin_ui_control_frame_create_by_res(control, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, str_value) == NULL) return -1;

    return 0;
}

int RGUIButton::down_res_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);

    if (str_value == NULL) return -1;

    plugin_ui_control_frame_clear_by_usage(control, plugin_ui_control_frame_usage_down, NULL);
    if (str_value[0]) {
        if (plugin_ui_control_frame_create_by_res(control, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, str_value) == NULL) return -1;
    }

    return 0;
}

int RGUIButton::add_back_down_res_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);

    if (str_value == NULL) return -1;

    return ((RGUIButton *)plugin_ui_control_product(control))->AddBackAndDownFrame(str_value, NULL);
}

int RGUIButton::back_down_res_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);

    if (str_value == NULL) return -1;

    return ((RGUIButton *)plugin_ui_control_product(control))->SetBackAndDownFrame(str_value, NULL);
}

void RGUIButton::setup(plugin_ui_control_meta_t meta) {
    RGUILabel::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_down, plugin_ui_event_scope_self, on_mouse_down);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_self, on_mouse_rise);

    plugin_ui_control_attr_meta_create(meta, "down-res", down_res_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "add-down-res", add_down_res_setter, NULL);

    plugin_ui_control_attr_meta_create(meta, "back-down-res", back_down_res_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "add-back-down-res", add_back_down_res_setter, NULL);
}
