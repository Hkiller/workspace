#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpepp/utils/MemBuffer.hpp"
#include "cpe/dr/dr_data_value.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/timer/TimerCenter.hpp"
#include "render/model/ui_data_sprite.h"
#include "render/utils/ui_transform.h"
#include "render/model/ui_data_layout.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_color.h"
#include "render/utils/ui_string_table.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render.h"
#include "plugin/editor/plugin_editor_editing.h"
#include "plugin/basicanim/plugin_basicanim_frame.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_touch_track.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin/ui/plugin_ui_control_attr_meta.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "RGUIControl.h"

RGUIControl::RGUIControl() {
}

RGUIControl::~RGUIControl(void) {
	DelChildren();
}

/* parent */
void RGUIControl::AddChild(RGUIControl* control, bool tail) {
    assert(control);
    
	//设值父控件
    if (tail) {
        plugin_ui_control_add_child_tail(this->control(), control->control());
    }
    else {
        plugin_ui_control_add_child_head(this->control(), control->control());
    }
}

void RGUIControl::DelChild(RGUIControl* control, bool destroy) {
	assert(control);

    plugin_ui_control_t remove_control = control->control();
    plugin_ui_control_remove_child(this->control(), remove_control);

	if (destroy) {
        plugin_ui_control_free(remove_control);
	}
}

void RGUIControl::DelChildren(bool destory) {
    if (destory) {
        plugin_ui_control_destory_childs(control());
    }
    else {
        plugin_ui_control_remove_childs(control());
    }
}

/*frame*/
plugin_ui_control_frame_t
RGUIControl::SetFrame( ui_runtime_render_obj_ref_t render_obj_ref, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, uint8_t pos_policy, plugin_ui_aspect_t aspect) {
    clear_frames(layer, usage, aspect);

    plugin_ui_control_frame_t frame = plugin_ui_control_frame_create(control(), layer, usage, render_obj_ref);
    if (frame && aspect) {
        plugin_ui_aspect_control_frame_add(aspect, frame, 1);
    }

    return frame;
}

plugin_ui_control_frame_t
RGUIControl::SetFrame(const char * res, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect) {
    clear_frames(layer, usage, aspect);

    if (res[0] == 0) return NULL;
    
    plugin_ui_control_frame_t frame = plugin_ui_control_frame_create_by_res(control(), layer, usage, res);
    if (frame && aspect) {
        plugin_ui_aspect_control_frame_add(aspect, frame, 1);
    }

    return frame;
}

plugin_ui_control_frame_t
RGUIControl::AddFrame( ui_runtime_render_obj_ref_t render_obj_ref, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, uint8_t pos_policy, plugin_ui_aspect_t aspect) {
    plugin_ui_control_frame_t frame = plugin_ui_control_frame_create(control(), layer, usage, render_obj_ref);
    if (frame && aspect) {
        plugin_ui_aspect_control_frame_add(aspect, frame, 1);
    }
    return frame;
}

plugin_ui_control_frame_t
RGUIControl::AddFrame(const char * res, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect) {
    plugin_ui_control_frame_t frame = plugin_ui_control_frame_create_by_res(control(), layer, usage, res);
    if (frame && aspect) {
        plugin_ui_aspect_control_frame_add(aspect, frame, 1);
    }
    return frame;
}

plugin_ui_control_frame_t
RGUIControl::AddFrame(UI_CONTROL_RES_REF const & res_ref, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect) {
    plugin_ui_control_frame_t frame = plugin_ui_control_frame_create_by_def(control(), layer, usage, &res_ref);
    if (frame && aspect) {
        plugin_ui_aspect_control_frame_add(aspect, frame, 1);
    }
    return frame;
}

void RGUIControl::ClearFrame(plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect) {
    clear_frames(layer, usage, aspect);
}

RColor RGUIControl::color(const char * str_color, ui_color const & dft_color) {
    ui_color color;
    if (ui_cache_find_color(plugin_ui_control_cache_mgr(control()), str_color, &color) == 0) {
        return color;
    }
    else {
        return dft_color;
    }
}

/*show & hide*/
void RGUIControl::Show(bool fireAnim) {
    SetVisible(true);
}

void RGUIControl::Hide(bool fireAnim) {
    SetVisible(false);
}

/*animation*/
Drow::Animation & RGUIControl::createAnim(const char * type_name, const char * name, plugin_ui_aspect_t aspect) {
    plugin_ui_animation_t anim = plugin_ui_animation_create_by_type_name(ui_env(), type_name);
    if (anim == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "RGUIControl::startAnim: anim type %s not exist!", type_name);
    }

    if (plugin_ui_animation_control_create(anim, control(), 1) == NULL) {
        plugin_ui_animation_free(anim);
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "RGUIControl::startAnim: tie to control fail!");
    }

    if (name) {
        if (plugin_ui_animation_set_name(anim, name) != 0) {
            plugin_ui_animation_free(anim);
            APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "RGUIControl::startAnim: anim set name %s fail!", name);
        }
    }

    if (aspect) {
        if (plugin_ui_aspect_animation_add(aspect, anim, 1) != 0) {
            plugin_ui_animation_free(anim);
            APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "RGUIControl::startAnim: anim add to aspect fail!");
        }
    }
    
    return *(Drow::Animation*)anim;
}

/*method*/
void RGUIControl::Retext(void) {
}

void RGUIControl::RetextRecur(void) {
	Retext();

    plugin_ui_control_it child_it;
    plugin_ui_control_childs(control(), &child_it);

    for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
        child;
        child = plugin_ui_control_it_next(&child_it))
    {
        RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
		c->RetextRecur();
    }
}

void RGUIControl::Load(ui_data_control_t control) {
}

void RGUIControl::Update( float deltaTime ) {
	/*更新自己 */
	UpdateSelf(deltaTime);
}

void RGUIControl::OnLoadProperty() {
}

ui_data_control_t RGUIControl::GetTemplate(uint32_t msg_id) {
    if (msg_id == 0) return NULL;
    const char * res = plugin_ui_control_msg(control(), msg_id);
    return GetTemplate(res);
}

ui_data_control_t RGUIControl::GetTemplate( const char * name ) {
    return plugin_ui_control_find_template(plugin_ui_env_module(plugin_ui_control_env(control())), name);
}

void RGUIControl::on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
}

void RGUIControl::on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
}

void RGUIControl::on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
}

void RGUIControl::UpdateSelf(float deltaTime) {
}

RGUIControl * RGUIControl::create(plugin_ui_page_t page, uint8_t type) {
    plugin_ui_control_t control = plugin_ui_control_create(page, type);
    assert(control);
    if (control == NULL) return NULL;

    return (RGUIControl*)(plugin_ui_control_product(control));
}

RGUIControl * RGUIControl::create(plugin_ui_page_t page, ui_data_control_t data_control) {
	RGUIControl * control = create(page, ui_data_control_type(data_control));
    assert(control);
    if (control == NULL) return NULL;

    if (plugin_ui_control_set_template(control->control(), data_control) != 0) {
        control->destory();
        return NULL;
    }

	assert(control);
    return control;
}

const char * RGUIControl::visibleMsg(uint32_t msg_id) const {
    return ui_string_table_message(plugin_ui_env_string_table(ui_env()), msg_id);
}

const char * RGUIControl::visibleMsg(uint32_t msg_id, char * args) const {
    return ui_string_table_message_format(plugin_ui_env_string_table(ui_env()), msg_id, args);
}

const char * RGUIControl::visiableTime(uint32_t msg_id, uint32_t t) const {
    return ui_string_table_message_format_time_local(plugin_ui_env_string_table(ui_env()), msg_id, t);
}
    
const char * RGUIControl::visiableTime(uint32_t msg_id, uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t sec, uint8_t min) const {
    return ui_string_table_message_format_time(plugin_ui_env_string_table(ui_env()), msg_id, year, mon, day, hour, sec, min);
}

const char * RGUIControl::visiableTimeDuration(uint32_t msg_id, int32_t time_diff) const {
    return ui_string_table_message_format_time_duration(plugin_ui_env_string_table(ui_env()), msg_id, time_diff);
}

const char * RGUIControl::visiableTimeDuration(uint32_t msg_id, uint32_t base, uint32_t v) const {
    return ui_string_table_message_format_time_duration_by_base(plugin_ui_env_string_table(ui_env()), msg_id, base, v);
}

RGUIControl * RGUIControl::create(uint32_t src_path_id) {
    if (src_path_id == 0) return NULL;
    const char * src_path = plugin_ui_control_msg(control(), src_path_id);
    return create(src_path);
}

RGUIControl * RGUIControl::create(plugin_ui_page_t page, const char * src_path) {
    plugin_ui_env_t env = plugin_ui_page_env(page);
    assert(env);

    ui_data_src_t layout_src = ui_data_src_find_by_path(plugin_ui_env_data_mgr(env), src_path, ui_data_src_type_layout);
    if (layout_src == NULL) {
        APP_CTX_ERROR(plugin_ui_page_app(page), "RGUIControl::create: res %s not exist!", src_path);
        return NULL;
    }

    ui_data_layout_t layout = (ui_data_layout_t)ui_data_src_product(layout_src);
    if (layout == NULL) {
        APP_CTX_ERROR(plugin_ui_page_app(page), "RGUIControl::create: res %s not loaded!", src_path);
        return NULL;
    }

    ui_data_control_t r = ui_data_layout_root(layout);
    if (r == NULL) {
        APP_CTX_ERROR(plugin_ui_page_app(page), "RGUIControl::create: res %s not root node!", src_path);
        return NULL;
    }

    return create(page, r);
}

int RGUIControl::back_res_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);

    if (str_value == NULL) return -1;

    RGUIControl::cast(control)->clear_frames(plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, NULL);
    if (str_value[0]) {
        if (plugin_ui_control_frame_create_by_res(control, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, str_value) == NULL) return -1;
    }
    
    return 0;
}

int RGUIControl::add_back_res_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);

    if (str_value == NULL) return -1;
    if (plugin_ui_control_frame_create_by_res(control, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, str_value) == NULL) return -1;
    
    return 0;
}

int RGUIControl::tail_res_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);

    if (str_value == NULL) return -1;

    RGUIControl::cast(control)->clear_frames(plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, NULL);
    if (str_value[0]) {
        if (plugin_ui_control_frame_create_by_res(control, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, str_value) == NULL) return -1;
    }
    
    return 0;
}

int RGUIControl::add_tail_res_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);

    if (str_value == NULL) return -1;
    if (plugin_ui_control_frame_create_by_res(control, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, str_value) == NULL) return -1;
    
    return 0;
}

int RGUIControl::color_setter(plugin_ui_control_t control, dr_value_t value) {
    uint32_t v;
    if (dr_value_try_read_uint32(&v, value, NULL) == 0) {
        RColor color = RColor(v);
        cast(control)->SetColor(color);
    }
    else if (const char * str_color = dr_value_try_read_string(value, NULL)) {
        ui_color color;
        if (ui_cache_find_color(plugin_ui_control_cache_mgr(control), str_color, &color) == 0) {
            cast(control)->SetColor(color);
        }
        else {
            return -1;
        }
    }
    else {
        return -1;
    }
    
    return 0;
}

int RGUIControl::enable_setter(plugin_ui_control_t control, dr_value_t value) {
    uint8_t v;
    if (dr_value_try_read_uint8(&v, value, NULL) != 0) return -1;
    plugin_ui_control_set_enable(control, v);
    return 0;
}

int RGUIControl::visible_setter(plugin_ui_control_t control, dr_value_t value) {
    uint8_t v;
    if (dr_value_try_read_uint8(&v, value, NULL) != 0) return -1;
    plugin_ui_control_set_visible(control, v);
    return 0;
}

int RGUIControl::alpha_setter(plugin_ui_control_t control, dr_value_t value) {
    float v;
    if (dr_value_try_read_float(&v, value, NULL) != 0) return -1;
    plugin_ui_control_set_alpha(control, v);
    return 0;
}

int RGUIControl::user_text_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);

    if (str_value == NULL) return -1;
    plugin_ui_control_set_user_text(control, str_value);
    
    return 0;
}

int RGUIControl::user_text_getter(plugin_ui_control_t control, dr_value_t data) {
    const char * user_text = plugin_ui_control_user_text(control);;
    
    data->m_type = CPE_DR_TYPE_STRING;
    data->m_meta = NULL;
    data->m_data = (void*)user_text;
    data->m_size = strlen(user_text) + 1;
        
    return 0;
}

int RGUIControl::scale_setter(plugin_ui_control_t control, dr_value_t value) {
    ui_vector_2 v;
    if (dr_value_try_read_float(&v.x, value, NULL) != 0) return -1;
    v.y = v.x;
    plugin_ui_control_set_scale(control, &v);
    return 0;
}

int RGUIControl::anim_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    if (dr_ctype_print_to_stream((write_stream_t)&s, value->m_data, value->m_type, NULL) < 0) {
        return -1;
    }
    stream_putc((write_stream_t)&s, 0);
    char * str_value = (char*)buffer.make_continuous();
    if (str_value == NULL) return -1;

    str_value = cpe_str_trim_head(str_value);

    while(char * sep = (char*)cpe_str_char_not_in_pair(str_value, ';', "{[(", ")]}")) {
        *sep = 0;

        plugin_ui_animation_t animation = plugin_ui_control_create_animation(control, str_value);
        if (animation == NULL) {
            APP_CTX_ERROR(
                plugin_ui_control_app(control), "control %s: create animation fail!",
                plugin_ui_control_name(control));
            return -1;
        }

        if (plugin_ui_animation_start(animation) != 0) {
            APP_CTX_ERROR(
                plugin_ui_control_app(control), "control %s: start animation fail!",
                plugin_ui_control_name(control));
            plugin_ui_animation_free(animation);
            return -1;
        }

        str_value = sep + 1;
    }

    if (str_value[0]) {
        plugin_ui_animation_t animation = plugin_ui_control_create_animation(control, str_value);
        if (animation == NULL) {
            APP_CTX_ERROR(
                plugin_ui_control_app(control), "control %s: create animation fail!",
                plugin_ui_control_name(control));
            return -1;
        }

        if (plugin_ui_animation_delay(animation) == 0.0f) {
            if (plugin_ui_animation_start(animation) != 0) {
                APP_CTX_ERROR(
                    plugin_ui_control_app(control), "control %s: start animation fail!",
                    plugin_ui_control_name(control));
                plugin_ui_animation_free(animation);
                return -1;
            }
        }
    }
    
    return 0;
}

int RGUIControl::frame_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    if (dr_ctype_print_to_stream((write_stream_t)&s, value->m_data, value->m_type, NULL) < 0) {
        return -1;
    }
    stream_putc((write_stream_t)&s, 0);
    char * str_value = (char*)buffer.make_continuous();
    if (str_value == NULL) return -1;

    if (const char * frame_name = cpe_str_read_and_remove_arg(str_value, "frame-name", ',', '=')) {
        plugin_ui_control_frame_t frame = plugin_ui_control_frame_find_by_name(control, frame_name);
        if (frame == NULL) {
            APP_CTX_ERROR(
                plugin_ui_control_app(control), "control %s: frame %s not exist!",
                plugin_ui_control_name(control), frame_name);
            return -1;
        }

        ui_runtime_render_obj_ref_set_args(plugin_ui_control_frame_render_obj_ref(frame), str_value);
    }

    if (const char * str_frame_layer = cpe_str_read_and_remove_arg(str_value, "frame-layer", ',', '=')) {
        plugin_ui_control_frame_layer_t layer;

        if (plugin_ui_control_frame_str_to_layer(str_frame_layer, &layer) != 0) {
            APP_CTX_ERROR(
                plugin_ui_control_app(control), "control %s: frame-layer %s unknown!",
                plugin_ui_control_name(control), str_frame_layer);
            return -1;
        }
        
        plugin_ui_control_frame_it frame_it;
        plugin_ui_control_frames_in_layer(control, &frame_it, layer);
        while(plugin_ui_control_frame_t frame = plugin_ui_control_frame_it_next(&frame_it)) {
            ui_runtime_render_obj_ref_set_args(plugin_ui_control_frame_render_obj_ref(frame), str_value);
        }
    }
    
    return 0;
}

void RGUIControl::setup(plugin_ui_control_meta_t meta) {
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_down, plugin_ui_event_scope_self, on_mouse_down);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_self, on_mouse_rise);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_click, plugin_ui_event_scope_self, on_mouse_click);

    plugin_ui_control_attr_meta_create(meta, "back-res", back_res_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "add-back-res", add_back_res_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "tail-res", tail_res_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "add-tail-res", add_tail_res_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "color", color_setter, NULL);  
    plugin_ui_control_attr_meta_create(meta, "enable", enable_setter, NULL);  
    plugin_ui_control_attr_meta_create(meta, "visible", visible_setter, NULL);  
    plugin_ui_control_attr_meta_create(meta, "alpha", alpha_setter, NULL);  
    plugin_ui_control_attr_meta_create(meta, "scale", scale_setter, NULL);  
    plugin_ui_control_attr_meta_create(meta, "user-text", user_text_setter, user_text_getter);  
    plugin_ui_control_attr_meta_create(meta, "anim", anim_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "frame", frame_setter, NULL);
}

void RGUIControl::SetRenderRealPTAbs( const RVector2& pt ) {
    ui_vector_2 t = pt;
    plugin_ui_control_set_render_pt_abs(control(), &t);
}

void RGUIControl::clear_frames(plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect) {
    struct plugin_ui_control_frame_it frame_it;
    plugin_ui_control_frame_t frame, next;
    
    plugin_ui_control_frames(control(), &frame_it);
    for(frame = plugin_ui_control_frame_it_next(&frame_it); frame; frame = next) {
        next = plugin_ui_control_frame_it_next(&frame_it);
        
        if (plugin_ui_control_frame_layer(frame) != layer) continue;
        if (plugin_ui_control_frame_usage(frame) != usage) continue;
        if (aspect && !plugin_ui_aspect_control_frame_is_in(aspect, frame)) continue;
        if (plugin_ui_aspect_control_frame_is_in(lock_aspect(), frame)) continue;

        plugin_ui_control_frame_free(frame);
    }
}

int RGUIControl::frame_rect_or_static(plugin_ui_control_frame_t frame, ui_rect & rect) {
    ui_vector_2_t size = plugin_ui_control_frame_base_size(frame);
    if (size->x == 0.0f || size->y == 0.0f) {
        ui_runtime_render_obj_t render_obj = plugin_ui_control_frame_render_obj(frame);
        
        switch(ui_runtime_render_obj_type_id(render_obj)) {
        case UI_OBJECT_TYPE_FRAME:
            if (ui_data_frame_bounding_rect(
                    plugin_basicanim_frame_data_frame((plugin_basicanim_frame_t)ui_runtime_render_obj_data(render_obj)), &rect)
                != 0)
            {
                APP_CTX_ERROR(
                    app(), "%s: frameOrStaticRect: calc frame size fail!",
                    plugin_ui_control_path_dump(app().tmpBuffer(), control()));
                return -1;
            }
            return 0;
        default:
            APP_CTX_ERROR(
                app(), "%s: frameOrStaticRect: no size!",
                plugin_ui_control_path_dump(app().tmpBuffer(), control()));
            return -1;
        }
    }
    else {
        rect.lt = *plugin_ui_control_frame_local_pos(frame);
        rect.rb.x = rect.lt.x + size->x;
        rect.rb.y = rect.lt.y + size->y;
        return 0;
    }
}

plugin_editor_module_t RGUIControl::editor_module(void) {
    return plugin_ui_module_editor(plugin_ui_env_module(ui_env()));
}

plugin_editor_editing_t
RGUIControl::create_editing(plugin_layout_render_t render) {
    return plugin_editor_editing_create(editor_module(), render);
}

