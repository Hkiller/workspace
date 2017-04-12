#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_layout.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin_ui_anim_control_anim_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_utils_i.h"

static int plugin_ui_anim_control_anim_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);

static uint8_t plugin_ui_anim_control_anim_update(
    plugin_ui_animation_t animation, void * ctx, float delta_s);

static void plugin_ui_anim_control_anim_update_to_frame(
    plugin_ui_anim_control_anim_t control_anim, plugin_ui_control_t control, uint8_t * control_updated);

plugin_ui_anim_control_anim_t
plugin_ui_anim_control_anim_create(plugin_ui_control_t control) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_ANIM);
    if (animation == NULL) return NULL;

    return plugin_ui_animation_data(animation);
}

plugin_ui_anim_control_anim_t
plugin_ui_anim_control_anim_create_with_setup(plugin_ui_control_t control, char * arg_buf_will_change) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_ANIM);
    if (animation == NULL) return NULL;

    if (plugin_ui_anim_control_anim_setup(animation, env->m_module, arg_buf_will_change, control, NULL)) {
        plugin_ui_animation_free(animation);
        return NULL;
    }

    return plugin_ui_animation_data(animation);
}

uint32_t plugin_ui_anim_control_anim_loop_count(plugin_ui_anim_control_anim_t anim) {
    return anim->m_loop_count;
}

void plugin_ui_anim_control_anim_set_loop_count(plugin_ui_anim_control_anim_t anim, uint32_t loop_count) {
    anim->m_loop_count = loop_count;
}

ui_data_control_anim_t
plugin_ui_anim_control_anim_data(plugin_ui_anim_control_anim_t control_anim) {
    return control_anim->m_anim_data;
}

int plugin_ui_anim_control_anim_set_data(
    plugin_ui_anim_control_anim_t control_anim, ui_data_control_anim_t anim_data)
{
    UI_CONTROL_ANIM const * d = ui_data_control_anim_data(anim_data);

    /*编辑器不支持设置Soft字段 */
    /* control_anim->m_soft = d->soft; */
    control_anim->m_loop_count = d->loop ? 0 : 1;
    control_anim->m_anim_data = anim_data;
    
    return 0;
}

void plugin_ui_anim_control_anim_reset(plugin_ui_anim_control_anim_t control_anim) {
    struct ui_data_control_anim_frame_it frame_it;
    ui_data_control_anim_frame_t anim_frame;

    assert(control_anim->m_anim_data);

    control_anim->m_cur_frame = -1;
    control_anim->m_total_frame = 0;
    control_anim->m_cur_trans_frame = NULL;
    control_anim->m_cur_scale_frame = NULL;
    control_anim->m_cur_alpha_frame = NULL;
    control_anim->m_cur_color_frame = NULL;
    control_anim->m_cur_angle_frame = NULL;

    ui_data_control_anim_frames(&frame_it, control_anim->m_anim_data);
    while((anim_frame = ui_data_control_anim_frame_it_next(&frame_it))) {
        UI_CONTROL_ANIM_FRAME const * anim_frame_data = ui_data_control_anim_frame_data(anim_frame);

        if (anim_frame_data->frame > control_anim->m_total_frame) {
            control_anim->m_total_frame = anim_frame_data->frame;
        }
        
        if (control_anim->m_next_trans_frame == NULL && anim_frame_data->have_trans) {
            control_anim->m_next_trans_frame = anim_frame;
        }

        if (control_anim->m_next_color_frame == NULL && anim_frame_data->have_color) {
            control_anim->m_next_color_frame = anim_frame;
        }
        
        if (control_anim->m_next_scale_frame == NULL && anim_frame_data->have_scale) {
            control_anim->m_next_scale_frame = anim_frame;
        }
        
        if (control_anim->m_next_alpha_frame == NULL && anim_frame_data->have_alpha) {
            control_anim->m_next_alpha_frame = anim_frame;
        }
        
        if (control_anim->m_next_angle_frame == NULL && anim_frame_data->have_angle) {
            control_anim->m_next_angle_frame = anim_frame;
        }
    }
}

static int plugin_ui_anim_control_anim_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_control_anim_t control_anim = plugin_ui_animation_data(animation);

    bzero(&control_anim->m_percent_decorator, sizeof(control_anim->m_percent_decorator));
    control_anim->m_soft = 1;
    control_anim->m_anim_data = NULL;
    control_anim->m_frame_time = 1.0f / 60.0f;
    control_anim->m_runing_time = 0.0f;
    control_anim->m_cur_trans_frame = NULL;
    control_anim->m_next_trans_frame = NULL;
    control_anim->m_cur_scale_frame = NULL;
    control_anim->m_next_scale_frame = NULL;
    control_anim->m_cur_alpha_frame = NULL;
    control_anim->m_next_alpha_frame = NULL;
    control_anim->m_cur_color_frame = NULL;
    control_anim->m_next_color_frame = NULL;
    control_anim->m_cur_angle_frame = NULL;
    control_anim->m_next_angle_frame = NULL;

    return 0;
}

static int plugin_ui_anim_control_anim_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_anim_t control_anim = plugin_ui_animation_data(animation);

    if (control_anim->m_anim_data == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_anim_enter: no animation data!");
        return -1;
    }

    plugin_ui_anim_control_anim_reset(control_anim);

    if (control_anim->m_total_frame <= 0) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_anim_enter: no frame!");
        return -1;
    }

    plugin_ui_anim_control_anim_update(animation, ctx, 0.0f) ;
    
    return 0;
}

static void plugin_ui_anim_control_anim_set_trans(
    plugin_ui_anim_control_anim_t control_anim, plugin_ui_control_t control,
    ui_data_control_anim_frame_t cur_frame, ui_data_control_anim_frame_t next_frame, float percent)
{
    ui_vector_2 pos;

    plugin_ui_calc_child(
        &pos,
        &ui_data_control_anim_frame_data(cur_frame)->trans.data,
        cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_ptls),
        &control->m_parent->m_render_sz_ns,
        &control->m_parent->m_editor_sz);
    
    if (percent > 0.0f) {
        ui_vector_2 next_pos;

        assert(next_frame);

        plugin_ui_calc_child(
            &next_pos,
            &ui_data_control_anim_frame_data(next_frame)->trans.data,
            cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_ptls),
            &control->m_parent->m_render_sz_ns,
            &control->m_parent->m_editor_sz);
        
        pos.x += (next_pos.x - pos.x) * percent;
        pos.y += (next_pos.y - pos.y) * percent;
    }

    plugin_ui_control_set_render_pt_by_real(control, &pos);
}

static void plugin_ui_anim_control_anim_set_color(
    plugin_ui_anim_control_anim_t control_anim, plugin_ui_control_t control,
    ui_data_control_anim_frame_t cur_frame, ui_data_control_anim_frame_t next_frame, float percent)
{
    ui_color color;
    UI_CONTROL_ANIM_FRAME const * cur_frame_data = ui_data_control_anim_frame_data(cur_frame);

    color.r = cur_frame_data->color.data.r;
    color.g = cur_frame_data->color.data.g;
    color.b = cur_frame_data->color.data.b;
    color.a = cur_frame_data->color.data.a;
    
    if (percent > 0.0f) {
        ui_color next_color;
        UI_CONTROL_ANIM_FRAME const * next_frame_data = ui_data_control_anim_frame_data(next_frame);

        next_color.r = next_frame_data->color.data.r;
        next_color.g = next_frame_data->color.data.g;
        next_color.b = next_frame_data->color.data.b;
        next_color.a = next_frame_data->color.data.a;
        
        color.r += (next_color.r - color.r) * percent;
        color.g += (next_color.g - color.g) * percent;
        color.b += (next_color.b - color.b) * percent;
        color.a += (next_color.b - color.a) * percent;
    }
    
    plugin_ui_control_set_color(control, &color);
}

static void plugin_ui_anim_control_anim_set_scale(
    plugin_ui_anim_control_anim_t control_anim, plugin_ui_control_t control,
    ui_data_control_anim_frame_t cur_frame, ui_data_control_anim_frame_t next_frame, float percent)
{
    ui_vector_2 scale;
    UI_CONTROL_ANIM_FRAME const * cur_frame_data = ui_data_control_anim_frame_data(cur_frame);

    scale.x = cur_frame_data->scale.data.value[0];
    scale.y = cur_frame_data->scale.data.value[1];
    
    if (percent > 0.0f) {
        UI_CONTROL_ANIM_FRAME const * next_frame_data = ui_data_control_anim_frame_data(next_frame);
        
        scale.x += (next_frame_data->scale.data.value[0] - scale.x) * percent;
        scale.y += (next_frame_data->scale.data.value[1] - scale.y) * percent;
    }

    plugin_ui_control_set_scale(control, &scale);
}

static void plugin_ui_anim_control_anim_set_alpha(
    plugin_ui_anim_control_anim_t control_anim, plugin_ui_control_t control,
    ui_data_control_anim_frame_t cur_frame, ui_data_control_anim_frame_t next_frame, float percent)
{
    float alpha = ui_data_control_anim_frame_data(cur_frame)->alpha.data;

    if (percent > 0.0f) {
        UI_CONTROL_ANIM_FRAME const * next_frame_data = ui_data_control_anim_frame_data(next_frame);
        
        alpha += (next_frame_data->alpha.data - alpha) * percent;
    }

    plugin_ui_control_set_alpha(control, alpha);
}

static void plugin_ui_anim_control_anim_set_angle(
    plugin_ui_anim_control_anim_t control_anim, plugin_ui_control_t control,
    ui_data_control_anim_frame_t cur_frame, ui_data_control_anim_frame_t next_frame, float percent)
{
    ui_vector_3 angle;
    UI_CONTROL_ANIM_FRAME const * cur_frame_data = ui_data_control_anim_frame_data(cur_frame);

    angle.x = cur_frame_data->angle.data.value[0];
    angle.y = cur_frame_data->angle.data.value[1];
    angle.z = cur_frame_data->angle.data.value[2];
    
    if (percent > 0.0f) {
        UI_CONTROL_ANIM_FRAME const * next_frame_data = ui_data_control_anim_frame_data(next_frame);
        
        angle.x += (next_frame_data->angle.data.value[0] - angle.x) * percent;
        angle.y += (next_frame_data->angle.data.value[1] - angle.y) * percent;
        angle.z += (next_frame_data->angle.data.value[2] - angle.z) * percent;
    }

    plugin_ui_control_set_angle(control, &angle);
}

static void plugin_ui_anim_control_anim_exit(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_control_anim_t control_anim = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return;

    /*退出时将状态推进到最后一帧 */
    if (control_anim->m_cur_frame != control_anim->m_total_frame) {
        uint8_t control_updated = 0;
        control_anim->m_cur_frame = control_anim->m_total_frame;
        plugin_ui_anim_control_anim_update_to_frame(control_anim, control, &control_updated);
    }
}

/*返回值表示是否需要处理 */
uint8_t plugin_ui_anim_control_anim_update_percent(
    plugin_ui_anim_control_anim_t control_anim,
    ui_data_control_anim_frame_t * cur_frame, ui_data_control_anim_frame_t * next_frame, float * percent)
{
    UI_CONTROL_ANIM_FRAME const * cur_frame_data;
    UI_CONTROL_ANIM_FRAME const * next_frame_data;
    uint8_t frame_changed = 0;

    while(*next_frame) {
        next_frame_data = ui_data_control_anim_frame_data(*next_frame);
        if (control_anim->m_cur_frame >= (int32_t)next_frame_data->frame) {
            *cur_frame = *next_frame;
            *next_frame = ui_data_control_anim_frame_next(*next_frame);
            frame_changed = 1;
            continue;
        }
        break;
    }
    
    /*无当前帧 */
    if (*cur_frame == NULL) return 0;

    /*不需要插值 */
    if (!control_anim->m_soft || (*next_frame == NULL)) {
        *percent = 0.0f;
        return frame_changed ? 1 : 0;
    }

    /*需要插值，计算插值值百分比 */
    cur_frame_data = ui_data_control_anim_frame_data(*cur_frame);
    assert(next_frame_data);

    if (next_frame_data->frame <= cur_frame_data->frame) {
        *percent = 0.0f;
        return 1;
    }

    assert(control_anim->m_cur_frame >= (int32_t)cur_frame_data->frame && control_anim->m_cur_frame < (int32_t)next_frame_data->frame);
    
    *percent =
        ((float)control_anim->m_cur_frame - (float)cur_frame_data->frame) /
        ((float)next_frame_data->frame - (float)cur_frame_data->frame);

    return 1;
}

static void plugin_ui_anim_control_anim_update_to_frame(
    plugin_ui_anim_control_anim_t control_anim, plugin_ui_control_t control, uint8_t * control_updated)
{
    float percent;
    
    /*位置 */
    if (plugin_ui_anim_control_anim_update_percent(
            control_anim, &control_anim->m_cur_trans_frame, &control_anim->m_next_trans_frame, &percent)
        )
    {
        if (!*control_updated) {
            plugin_ui_control_check_update_from_root(control);
            *control_updated = 1;
        }
        
        plugin_ui_anim_control_anim_set_trans(
            control_anim, control, control_anim->m_cur_trans_frame, control_anim->m_next_trans_frame, percent);
    }

    /*颜色 */
    if (plugin_ui_anim_control_anim_update_percent(
            control_anim, &control_anim->m_cur_color_frame, &control_anim->m_next_color_frame, &percent)
        )
    {
        if (!*control_updated) {
            plugin_ui_control_check_update_from_root(control);
            *control_updated = 1;
        }
        
        plugin_ui_anim_control_anim_set_color(
            control_anim, control, control_anim->m_cur_color_frame, control_anim->m_next_color_frame, percent);
    }

    /*缩放 */
    if (plugin_ui_anim_control_anim_update_percent(
            control_anim, &control_anim->m_cur_scale_frame, &control_anim->m_next_scale_frame, &percent)
        )
    {
        if (!*control_updated) {
            plugin_ui_control_check_update_from_root(control);
            *control_updated = 1;
        }

        plugin_ui_anim_control_anim_set_scale(
            control_anim, control, control_anim->m_cur_scale_frame, control_anim->m_next_scale_frame, percent);
    }

    /*Alpha */
    if (plugin_ui_anim_control_anim_update_percent(
            control_anim, &control_anim->m_cur_alpha_frame, &control_anim->m_next_alpha_frame, &percent)
        )
    {
        if (!*control_updated) {
            plugin_ui_control_check_update_from_root(control);
            *control_updated = 1;
        }

        plugin_ui_anim_control_anim_set_alpha(
            control_anim, control, control_anim->m_cur_alpha_frame, control_anim->m_next_alpha_frame, percent);
    }

    /*angle */
    if (plugin_ui_anim_control_anim_update_percent(
            control_anim, &control_anim->m_cur_angle_frame, &control_anim->m_next_angle_frame, &percent)
        )
    {
        if (!*control_updated) {
            plugin_ui_control_check_update_from_root(control);
            *control_updated = 1;
        }

        plugin_ui_anim_control_anim_set_angle(
            control_anim, control, control_anim->m_cur_angle_frame, control_anim->m_next_angle_frame, percent);
    }
}

static uint8_t plugin_ui_anim_control_anim_update(plugin_ui_animation_t animation, void * ctx, float delta) {
    plugin_ui_anim_control_anim_t control_anim = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    int32_t cur_frame;
    float total_time;
    uint8_t control_updated = 0;

    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return 0;

    control_anim->m_runing_time += delta;
    cur_frame = (int32_t)(control_anim->m_runing_time / control_anim->m_frame_time);

    if (cur_frame == control_anim->m_cur_frame) return 1;

    if (cur_frame < (int32_t)control_anim->m_total_frame) { /*循环还没有完成，则直接更新 */
        control_anim->m_cur_frame = cur_frame;
        plugin_ui_anim_control_anim_update_to_frame(control_anim, control, &control_updated);
        return 1;
    }

    /*一次循环完成 */
    control_anim->m_cur_frame = control_anim->m_total_frame;
    plugin_ui_anim_control_anim_update_to_frame(control_anim, control, &control_updated);

    if (control_anim->m_loop_count > 0) {
        control_anim->m_loop_count--;
        if (control_anim->m_loop_count == 0) return 0; /*循环完成则停止动画 */
    }

    /*帧数重置到循环后的状态 */
    plugin_ui_anim_control_anim_reset(control_anim);
    total_time = control_anim->m_total_frame * control_anim->m_frame_time;
    while(control_anim->m_runing_time >= total_time) control_anim->m_runing_time -= total_time;
    control_anim->m_cur_frame = cur_frame % control_anim->m_total_frame;

    /**/
    plugin_ui_anim_control_anim_update_to_frame(control_anim, control, &control_updated);

    return 1;
}

static int plugin_ui_anim_control_anim_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_anim_t control_anim = plugin_ui_animation_data(animation);
    char * str_value;

    if (plugin_ui_animation_control_create(animation, control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_anim_setup: create animation control fail!");
        return -1;
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-anim.anim", ',', '='))) {
        uint8_t anim_type;
        ui_data_control_anim_t anim_data = NULL;

        if (strcmp(str_value, "show") == 0) {
            anim_type = ui_control_anim_type_show;
        }
        else if (strcmp(str_value, "dead") == 0) {
            anim_type = ui_control_anim_type_dead;
        }
        else if (strcmp(str_value, "down") == 0) {
            anim_type = ui_control_anim_type_down;
        }
        else if (strcmp(str_value, "rise") == 0) {
            anim_type = ui_control_anim_type_rise;
        }
        else if (strcmp(str_value, "user") == 0) {
            anim_type = ui_control_anim_type_user;
        }
        else {
            CPE_ERROR(module->m_em, "plugin_ui_anim_control_anim_setup: unknown anim %s!", str_value);
            return -1;
        }
        
        anim_data = plugin_ui_control_find_anim_data(control, anim_type);
        if (anim_data == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_control_anim_setup: anim %s not exist!", str_value);
            return -1;
        }

        if (plugin_ui_anim_control_anim_set_data(control_anim, anim_data) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_control_anim_setup: anim %s set fail!", str_value);
            return -1;
        }
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-anim.loop-count", ',', '='))) {
        control_anim->m_loop_count = atoi(str_value);
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-anim.soft", ',', '='))) {
        control_anim->m_soft = atoi(str_value) ? 1 : 0;
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "fps", ',', '='))) {
        float fps = atof(str_value);
        if (fps <= 0.0f) {
            CPE_ERROR(module->m_em, "control-basic-atof: fps %s error!", str_value);
            return -1;
        }
        else {
            control_anim->m_frame_time = 1.0f / fps;
        }
    }
    
    return 0;
}

int plugin_ui_anim_control_anim_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module, PLUGIN_UI_ANIM_CONTROL_ANIM, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_control_anim),
            plugin_ui_anim_control_anim_init,
            NULL,
            plugin_ui_anim_control_anim_enter,
            plugin_ui_anim_control_anim_exit,
            plugin_ui_anim_control_anim_update,
            /*control*/
            0, NULL, NULL,
            /*setup*/
            plugin_ui_anim_control_anim_setup);
    return meta ? 0 : -1;
}

void plugin_ui_anim_control_anim_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta = plugin_ui_animation_meta_find(module, PLUGIN_UI_ANIM_CONTROL_ANIM);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_CONTROL_ANIM = "control-anim";
