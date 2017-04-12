#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data_value.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin_ui_anim_progress_bind_control_i.h"

plugin_ui_anim_progress_bind_control_t
plugin_ui_anim_progress_bind_control_create(plugin_ui_env_t env) {
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_PROGRESS_BIND_CONTROL_NAME);
    if (animation == NULL) return NULL;
    
    return (plugin_ui_anim_progress_bind_control_t)plugin_ui_animation_data(animation);
}

int plugin_ui_anim_progress_bind_control_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_progress_bind_control_t anim_bind_control = plugin_ui_animation_data(animation);
    anim_bind_control->m_module = module;
    anim_bind_control->m_pos_policy = plugin_ui_anim_progress_bind_control_near;
    return 0;
}

void plugin_ui_anim_progress_bind_control_free(plugin_ui_animation_t animation, void * ctx) {
}

static int plugin_ui_anim_progress_bind_control_update_controls(
    plugin_ui_module_t module, plugin_ui_control_t progress, plugin_ui_animation_t animation)
{
    plugin_ui_anim_progress_bind_control_t anim_bind_control = plugin_ui_animation_data(animation);
    struct plugin_ui_control_it control_it;
    struct dr_value value_buf;
    ui_vector_2_t progress_pt;
    ui_vector_2_t progress_sz;
    float progress_progress;
    uint8_t progress_mode;
    plugin_ui_control_t bind_to;
    
    progress_pt = plugin_ui_control_real_pt_abs(progress);
    ui_assert_vector_2_sane(progress_pt);

    progress_sz = plugin_ui_control_real_sz_abs(progress);
    ui_assert_vector_2_sane(progress_sz);

    if (plugin_ui_control_get_attr(progress, "progress", &value_buf) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_ui_anim_progress_bind_control: control %s read progress fail!",
            plugin_ui_control_name(progress));
        return -1;
    }
    progress_progress = dr_value_read_with_dft_float(&value_buf, 0.0f);

    if (plugin_ui_control_get_attr(progress, "mode", &value_buf) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_ui_anim_progress_bind_control: control %s read mode fail!",
            plugin_ui_control_name(progress));
        return -1;
    }
    progress_mode = dr_value_read_with_dft_uint8(&value_buf, 0);

    /*遍历放置绑定控件 */
    plugin_ui_animation_controls(animation, &control_it);
    while((bind_to = plugin_ui_control_it_next(&control_it))) {
        ui_vector_2 bind_to_pt;
        ui_vector_2_t bind_to_sz;
        
        if (bind_to == progress) continue;

        bind_to_pt = * plugin_ui_control_real_pt_abs(bind_to);
        ui_assert_vector_2_sane(&bind_to_pt);

        bind_to_sz = plugin_ui_control_real_sz_abs(bind_to);
        
        switch(progress_mode) {
        case 0/*PM_XINC*/:
            bind_to_pt.x = progress_pt->x + progress_sz->x * progress_progress -  bind_to_sz->x + bind_to_sz->x / 2.0f * anim_bind_control->m_pos_policy;
            break;
        case 1/*PM_XINC_R*/:
            break;
        case 2/*PM_XDEC*/:
            break;
        case 3/*PM_XDEC_R*/:
            break;
        case 4/*PM_YINC*/:
            bind_to_pt.y = progress_pt->y + progress_sz->y * progress_progress - bind_to_sz->y + bind_to_sz->y / 2.0f * anim_bind_control->m_pos_policy;
            break;
        case 5/*PM_YINC_R*/:
            break;
        case 6/*PM_YDEC*/:
            break;
        case 7/*PM_YDEC_R*/:
            break;
        }

        plugin_ui_control_set_render_pt_by_real(bind_to, &bind_to_pt);
    }

    return 0;
}

int plugin_ui_anim_progress_bind_control_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = (plugin_ui_module_t)ctx;
    plugin_ui_control_t progress_control;

    progress_control = plugin_ui_animation_find_first_tie_control(animation);
    if (progress_control == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_progress_bind_control: progress control not exist!");
        return -1;
    }
    
    plugin_ui_anim_progress_bind_control_update_controls(module, progress_control, animation);
    return 0;
}

void plugin_ui_anim_progress_bind_control_exit(plugin_ui_animation_t animation, void * ctx) {
}

uint8_t plugin_ui_anim_progress_bind_control_update(plugin_ui_animation_t animation, void * ctx, float delta_s) {
    plugin_ui_module_t module = (plugin_ui_module_t)ctx;
    plugin_ui_control_t progress_control;

    progress_control = plugin_ui_animation_find_first_tie_control(animation);
    if (progress_control == NULL) return 0;

    plugin_ui_anim_progress_bind_control_update_controls(module, progress_control, animation);

    return 1;
}

int plugin_ui_anim_progress_bind_control_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_anim_progress_bind_control_t anim_bind_control = (plugin_ui_anim_progress_bind_control_t)plugin_ui_animation_data(animation);
    plugin_ui_module_t module = anim_bind_control->m_module;
    const char * str_value;
    plugin_ui_page_t page = plugin_ui_control_page(control);

    if (plugin_ui_animation_control_create(animation, control, 1) == NULL) {
        CPE_ERROR(anim_bind_control->m_module->m_em, "plugin_ui_anim_progress_bind_control: add main control fail!");
        return -1;
    }
    
    while ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "bind-to", ',', '='))) {
        plugin_ui_control_t bind_to =
            plugin_ui_control_find_child_by_name_no_category_r(plugin_ui_page_root_control(page), str_value);
        if (bind_to == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_progress_bind_control: bind-to control %s not exist!", str_value);
            return -1;
        }
        
        if (plugin_ui_animation_control_create(animation, bind_to, 0) == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_progress_bind_control: add bind-to control %s fail!", str_value);
            return -1;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "bind-pos", ',', '='))) {
        if (strcmp(str_value, "near") == 0) {
            anim_bind_control->m_pos_policy = plugin_ui_anim_progress_bind_control_near;
        }
        else if (strcmp(str_value, "middle") == 0) {
            anim_bind_control->m_pos_policy = plugin_ui_anim_progress_bind_control_middle;
        }
        else if (strcmp(str_value, "far") == 0) {
            anim_bind_control->m_pos_policy = plugin_ui_anim_progress_bind_control_far;
        }
        else {
            CPE_ERROR(module->m_em, "plugin_ui_anim_progress_bind_control: bind-pos %s unknown, expect near\\middle\\far!", str_value);
        }
    }
    
    return 0;
}

int plugin_ui_anim_progress_bind_control_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module,
            PLUGIN_UI_ANIM_PROGRESS_BIND_CONTROL_NAME, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_progress_bind_control),
            plugin_ui_anim_progress_bind_control_init,
            plugin_ui_anim_progress_bind_control_free,
            plugin_ui_anim_progress_bind_control_enter,
            plugin_ui_anim_progress_bind_control_exit,
            plugin_ui_anim_progress_bind_control_update,
            /*control*/
            0, NULL, NULL,
            plugin_ui_anim_progress_bind_control_setup);
    
    return meta ? 0 : -1;
}

void plugin_ui_anim_progress_bind_control_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_find(
            module,
            PLUGIN_UI_ANIM_PROGRESS_BIND_CONTROL_NAME);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_PROGRESS_BIND_CONTROL_NAME = "progress-bind-control";
