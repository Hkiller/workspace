#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_sound_group.h"
#include "plugin/ui/plugin_ui_module.h"
#include "plugin/ui/plugin_ui_package_queue_managed.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_cfg.h"
#include "plugin/ui/plugin_ui_page_plugin.h"
#include "plugin/ui/plugin_ui_page_meta.h"
#include "plugin/ui/plugin_ui_phase_use_page.h"
#include "plugin/ui/plugin_ui_phase_use_popup_def.h"
#include "plugin/ui/plugin_ui_control_category.h"
#include "plugin/ui/plugin_ui_control_binding.h"
#include "plugin/ui/plugin_ui_popup_def.h"
#include "plugin/ui/plugin_ui_popup_def_binding.h"
#include "plugin/ui/plugin_ui_popup_def_binding_attr.h"
#include "ui_sprite_ui_env_i.h"
#include "ui_sprite_ui_phase_i.h"
#include "ui_sprite_ui_popup_def_i.h"
#include "ui_sprite_ui_page_plugin_anim_i.h"
#include "ui_sprite_ui_page_plugin_bindings_i.h"

static int ui_sprite_ui_env_load_package_queues(ui_sprite_ui_env_t env, cfg_t cfg);
static int ui_sprite_ui_env_load_sound_cfg(ui_sprite_ui_env_t env, cfg_t cfg);
static int ui_sprite_ui_env_load_control_categories(ui_sprite_ui_env_t env, cfg_t cfg);
static int ui_sprite_ui_env_load_phases(ui_sprite_ui_env_t env, cfg_t cfg);
static int ui_sprite_ui_env_load_pages(ui_sprite_ui_env_t env, cfg_t cfg);
static int ui_sprite_ui_env_load_popups(ui_sprite_ui_env_t env, cfg_t cfg);
static int ui_sprite_ui_env_load_init_phase(ui_sprite_ui_env_t env, cfg_t cfg);
static int ui_sprite_ui_env_load_defaults(ui_sprite_ui_env_t env, cfg_t cfg);

int ui_sprite_ui_env_load(ui_sprite_ui_env_t env, cfg_t cfg, const char * str_language) {
    const char * str_value;
    
    ui_sprite_ui_env_set_debug(env, cfg_get_uint8(cfg, "ui-center.debug", plugin_ui_env_debug(env->m_env)));
    //ui_sprite_ui_env_set_update_priority(env, cfg_get_uint8(cfg, "ui-center.update-priority", +1));

    plugin_ui_env_set_accept_multi_touch(
        env->m_env, cfg_get_float(cfg, "ui-center.accept-multi-touch", plugin_ui_env_accept_multi_touch(env->m_env)));

    if ((str_value = cfg_get_string(cfg, "ui-center.resize-policy", NULL))) {
        if (strcmp(str_value, "by-width") == 0) {
            plugin_ui_env_set_screen_resize_policy(env->m_env, plugin_ui_screen_resize_by_width);
        }
        else if (strcmp(str_value, "by-height") == 0) {
            plugin_ui_env_set_screen_resize_policy(env->m_env, plugin_ui_screen_resize_by_height);
        }
        else if (strcmp(str_value, "free") == 0) {
            plugin_ui_env_set_screen_resize_policy(env->m_env, plugin_ui_screen_resize_free);
        }
        else {
            CPE_ERROR(env->m_module->m_em, "ui-center.resize-policy %s unknown", str_value);
            return -1;
        }
    }
    
    if (plugin_ui_env_set_language(env->m_env, str_language) != 0
        || ui_sprite_ui_env_load_package_queues(env, cfg_find_cfg(cfg, "env.package-queues")) != 0
        || ui_sprite_ui_env_load_sound_cfg(env, cfg_find_cfg(cfg, "env.sound")) != 0
        || ui_sprite_ui_env_load_control_categories(env, cfg_find_cfg(cfg, "ui-center.control-categories")) != 0
        || ui_sprite_ui_env_load_phases(env, cfg_find_cfg(cfg, "phases")) != 0
        || ui_sprite_ui_env_load_pages(env, cfg_find_cfg(cfg, "pages")) != 0
        || ui_sprite_ui_env_load_popups(env, cfg_find_cfg(cfg, "popups")) != 0
        || ui_sprite_ui_env_load_init_phase(env, cfg) != 0
        || ui_sprite_ui_env_load_defaults(env, cfg_find_cfg(cfg, "defaults")) != 0
        )
    {
        return -1;
    }

    return 0;
}

static int ui_sprite_ui_env_load_phases(ui_sprite_ui_env_t env, cfg_t cfg) {
    struct cfg_it cfg_it;
    cfg_t child_cfg;
    ui_sprite_cfg_loader_t loader;

    loader = ui_sprite_cfg_loader_find_nc(env->m_module->m_app, NULL);
    if (loader == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: find cfg load fail!");
        return -1;
    }

    /*加载所有phase数据，除了跳转关系 */
    cfg_it_init(&cfg_it, cfg);
    while((child_cfg = cfg_it_next(&cfg_it))) {
        const char * phase_name = cfg_name(child_cfg);
        plugin_ui_phase_t b_phase;

        b_phase = plugin_ui_phase_create(env->m_env, phase_name);
        if (b_phase == NULL) {
            CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: phase %s: create fail!", phase_name);
            return -1;
        }

        if (ui_sprite_ui_env_phase_load_without_navigations(env, b_phase, loader, child_cfg) != 0) {
            plugin_ui_phase_free(b_phase);
            return -1;
        }

        if (plugin_ui_env_debug(env->m_env)) {
            CPE_INFO(env->m_module->m_em, "ui_sprite_ui_env_load: phase %s: load success!", phase_name);
        }
    }

    /*加载所有跳转关系 */
    cfg_it_init(&cfg_it, cfg);
    while((child_cfg = cfg_it_next(&cfg_it))) {
        const char * phase_name = cfg_name(child_cfg);
        plugin_ui_phase_t b_phase;

        b_phase = plugin_ui_phase_find(env->m_env, phase_name);
        if (b_phase == NULL) continue;

        if (ui_sprite_ui_env_phase_load_navigations(env, b_phase, child_cfg) != 0) {
            return -1;
        }

        if (plugin_ui_env_debug(env->m_env)) {
            CPE_INFO(env->m_module->m_em, "ui_sprite_ui_env_load: phase %s: load navigations success!", phase_name);
        }
    }

    return 0;
}

static int ui_sprite_ui_env_load_control_categories(ui_sprite_ui_env_t env, cfg_t cfg) {
    struct cfg_it cfg_it;
    cfg_t child_cfg;

    cfg_it_init(&cfg_it, cfg);
    while((child_cfg = cfg_it_next(&cfg_it))) {
        const char * postfix = cfg_get_string(child_cfg, "postfix", NULL);
        const char * str_value;
        plugin_ui_control_category_t category;

        if (postfix == NULL) {
            CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: category: format error!");
            return -1;
        }

        category = plugin_ui_control_category_create(env->m_env, postfix);
        if (category == NULL) {
            CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: category %s: create fail!", postfix);
            return -1;
        }

        if ((str_value = cfg_get_string(child_cfg, "click-audio", NULL))) {
            if (plugin_ui_control_category_set_click_audio(category, str_value) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "ui_sprite_ui_env_load: category %s: set click-audio %s fail!",
                    postfix, str_value);
                return -1;
            }
        }

        if (plugin_ui_env_debug(env->m_env)) {
            CPE_INFO(env->m_module->m_em, "ui_sprite_ui_env_load: category %s: success!", postfix);
        }
    }

    return 0;
}

static int ui_sprite_ui_env_load_pages(ui_sprite_ui_env_t env, cfg_t cfg) {
    struct cfg_it cfg_it;
    cfg_t child_cfg;
    plugin_ui_module_t ui_module;

    ui_module = plugin_ui_env_module(env->m_env);
    
    cfg_it_init(&cfg_it, cfg);
    while((child_cfg = cfg_it_next(&cfg_it))) {
        const char * page_name = cfg_name(child_cfg);
        const char * page_type = cfg_get_string(child_cfg, "type", page_name);
        const char * load_from = cfg_get_string(child_cfg, "load-from", NULL);
        const char * load_policy = cfg_get_string(child_cfg, "load-policy", NULL);
        plugin_ui_page_meta_t page_meta;
        plugin_ui_page_t page;
        struct cfg_it join_phase_it;
        cfg_t join_phase_cfg;
        struct cfg_it cfg_binding_it;
        cfg_t cfg_binding;
        struct cfg_it cfg_anim_it;
        cfg_t cfg_anim;

        page_meta = plugin_ui_page_meta_find(ui_module, page_type);
        if (page_meta == NULL) {
            page_meta = plugin_ui_page_meta_load_from_module(ui_module, page_type);
            if (page_meta == NULL) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: page %s: load page meta from %s fail!", page_name, page_type);
                return -1;
            }
        }

        page = plugin_ui_page_create(env->m_env, page_name, page_meta);
        if (page == NULL) {
            CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: page %s: create page from %s fail!", page_name, page_type);
            return -1;
        }   

        if (cfg_get_uint8(child_cfg, "force-change", 0)) {
            plugin_ui_page_set_force_change(page, 1);
        }
        
        if (load_from) {
            if (plugin_ui_page_set_src_by_path(page, load_from) != 0) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: page %s: set page load-from %s fail!", page_name, load_from);
                return -1;
            }
        }

        if (load_policy) {
            if (strcmp(load_policy, "phase") == 0) {
                plugin_ui_page_set_load_policy(page, plugin_ui_page_load_policy_phase);
            }
            else if (strcmp(load_policy, "visiable") == 0) {
                plugin_ui_page_set_load_policy(page, plugin_ui_page_load_policy_visiable);
            }
            else {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: page %s: load-policy %s unknown!", page_name, load_policy);
                return -1;
            }
        }
        
        cfg_it_init(&join_phase_it, cfg_find_cfg(child_cfg, "phases"));
        while((join_phase_cfg = cfg_it_next(&join_phase_it))) {
            const char * phase_name = cfg_as_string(join_phase_cfg, NULL);
            plugin_ui_phase_t phase;
            plugin_ui_phase_use_page_t use_page;

            if (phase_name == NULL) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: page %s: phases config format error!", page_name);
                return -1;
            }

            phase = plugin_ui_phase_find(env->m_env, phase_name);
            if (phase == NULL) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: page %s: use phase %s not exist!", page_name, phase_name);
                return -1;
            }

            use_page = plugin_ui_phase_use_page_create(phase, page);
            if (use_page == NULL) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: page %s: phase %s: create use page not exist!", page_name, phase_name);
                return -1;
            }
        }

        cfg_it_init(&cfg_anim_it, cfg_find_cfg(child_cfg, "animations"));
        while((cfg_anim = cfg_it_next(&cfg_anim_it))) {
            if (ui_sprite_ui_page_plugin_anim_create(env, page, cfg_anim) != 0) return -1;
        }

        cfg_it_init(&cfg_binding_it, cfg_find_cfg(child_cfg, "bindings"));
        while((cfg_binding = cfg_it_next(&cfg_binding_it))) {
            if (ui_sprite_ui_page_plugin_bindings_load(env, page, cfg_binding) != 0) return -1;
        }
    }

    return 0;
}

static int ui_sprite_ui_env_load_init_phase(ui_sprite_ui_env_t env, cfg_t cfg) {
    const char * str_value;
    
    if ((str_value = cfg_get_string(cfg, "ui-center.init-phase", NULL))) {
        if (plugin_ui_env_set_init_phase(
                env->m_env, str_value, cfg_get_string(cfg, "ui-center.init-call-phase", NULL)) != 0)
        {
            CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: set init-phase fail!");
            return -1;
        }
    }
    else {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: init-phase not configured!");
        return -1;
    }

    return 0;
}

static int ui_sprite_ui_env_load_popup_binding(ui_sprite_ui_env_t env, plugin_ui_popup_def_t popup_def, cfg_t cfg) {
    const char * control;
    plugin_ui_popup_def_binding_t binding;
    struct cfg_it attr_it;
    cfg_t attr_cfg;

    if ((control = cfg_get_string(cfg, "control", NULL)) == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_load_popups: create popup %s: binding control not configured!",
            plugin_ui_popup_def_name(popup_def));
        return -1;
    }

    if ((binding = plugin_ui_popup_def_binding_create(popup_def, control)) == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_load_popups: create popup %s: crate binding of control %s fail!",
            plugin_ui_popup_def_name(popup_def), control);
        return -1;
    }

    cfg_it_init(&attr_it, cfg);
    while((attr_cfg = cfg_it_next(&attr_it))) {
        const char * name = cfg_name(attr_cfg);

        if (strcmp(name, "control") == 0) {
            continue;
        }
        else if (strcmp(name, "on-click-close") == 0) {
            plugin_ui_popup_def_binding_set_on_click_close(binding, cfg_as_uint8(attr_cfg, 0));
        }
        else if (strcmp(name, "on-click-action") == 0) {
            const char * value = cfg_as_string(attr_cfg, NULL);

            if (value == NULL) {
                CPE_ERROR(
                    env->m_module->m_em,
                    "ui_sprite_ui_env_load_popups: create popup %s: crate binding of control %s: on-click-action format error!",
                    plugin_ui_popup_def_name(popup_def), control);
                return -1;
            }
            
            if (plugin_ui_popup_def_binding_set_on_click_action(binding, value) != 0) {
                CPE_ERROR(
                    env->m_module->m_em,
                    "ui_sprite_ui_env_load_popups: create popup %s: crate binding of control %s: set on-click-action %s fail!",
                    plugin_ui_popup_def_name(popup_def), control, value);
                return -1;
            }
        }
        else {
            const char * value = cfg_as_string(attr_cfg, NULL);

            if (value == NULL) {
                CPE_ERROR(
                    env->m_module->m_em,
                    "ui_sprite_ui_env_load_popups: create popup %s: crate binding of control %s: attr %s value config format error!",
                    plugin_ui_popup_def_name(popup_def), control, name);
                return -1;
            }
            
            if (plugin_ui_popup_def_binding_attr_create(binding, name, value) == NULL) {
                CPE_ERROR(
                    env->m_module->m_em,
                    "ui_sprite_ui_env_load_popups: create popup %s: crate binding of control %s: attr %s value %s create fail!",
                    plugin_ui_popup_def_name(popup_def), control, name, value);
                return -1;
            }
        }
    }

    return 0;
}

static int ui_sprite_ui_env_load_popups(ui_sprite_ui_env_t env, cfg_t cfg) {
    struct cfg_it cfg_it;
    cfg_t child_cfg;
    plugin_ui_module_t ui_module;

    ui_module = plugin_ui_env_module(env->m_env);
    
    cfg_it_init(&cfg_it, cfg);
    while((child_cfg = cfg_it_next(&cfg_it))) {
        const char * popup_name = cfg_name(child_cfg);
        const char * str_value;
        cfg_t cfg_value;
        plugin_ui_popup_def_t popup_def;
        struct cfg_it cfg_binding_it;
        cfg_t cfg_binding;
        struct cfg_it join_phase_it;
        cfg_t join_phase_cfg;

        popup_def = plugin_ui_popup_def_create(env->m_env, popup_name);
        if (popup_def == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_load_popups: create popup %s fail!",
                popup_name);
            return -1;
        }

        if ((str_value = cfg_get_string(child_cfg, "type", NULL))) {
            plugin_ui_page_meta_t page_meta;
            
            page_meta = plugin_ui_page_meta_find(ui_module, str_value);
            if (page_meta == NULL) {
                page_meta = plugin_ui_page_meta_load_from_module(ui_module, str_value);
                if (page_meta == NULL) {
                    CPE_ERROR(
                        env->m_module->m_em, "ui_sprite_ui_env_load_popups: create popup %s: load page meta from %s fail!", popup_name, str_value);
                    return -1;
                }
            }

            plugin_ui_popup_def_set_page_meta(popup_def, page_meta);
        }
        
        if ((str_value = cfg_get_string(child_cfg, "load-from", NULL))) {
            if (plugin_ui_popup_def_set_page_load_from_by_path(popup_def, str_value) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "ui_sprite_ui_env_load_popups: create popup %s: set load-from %s fail!",
                    popup_name, str_value);
                return -1;
            }
        }

        if ((cfg_value = cfg_find_cfg(child_cfg, "layer"))) {
            plugin_ui_popup_def_set_layer(popup_def, cfg_as_int16(cfg_value, plugin_ui_popup_def_layer(popup_def)));
        }

        if ((cfg_value = cfg_find_cfg(child_cfg, "duration"))) {
            plugin_ui_popup_def_set_lifecircle(popup_def, cfg_as_float(cfg_value, 0.0f));
        }

        cfg_it_init(&cfg_binding_it, cfg_find_cfg(child_cfg, "bindings"));
        while((cfg_binding = cfg_it_next(&cfg_binding_it))) {
            if (ui_sprite_ui_env_load_popup_binding(env, popup_def, cfg_binding) != 0) return -1;
        }

        cfg_it_init(&join_phase_it, cfg_find_cfg(cfg, "phases"));
        while((join_phase_cfg = cfg_it_next(&join_phase_it))) {
            const char * phase_name = cfg_as_string(join_phase_cfg, NULL);
            plugin_ui_phase_t phase;
            plugin_ui_phase_use_popup_def_t use_popup_def;

            if (phase_name == NULL) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: popup_def %s: phases config format error!", popup_name);
                return -1;
            }

            phase = plugin_ui_phase_find(env->m_env, phase_name);
            if (phase == NULL) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: popup_def %s: use phase %s not exist!", popup_name, phase_name);
                return -1;
            }

            use_popup_def = plugin_ui_phase_use_popup_def_create(phase, popup_def);
            if (use_popup_def == NULL) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: popup_def %s: phase %s: create use popup_def not exist!", popup_name, phase_name);
                return -1;
            }
        }

        if ((cfg_value = cfg_find_cfg(child_cfg, "runing-fsm"))) {
            ui_sprite_cfg_loader_t loader = ui_sprite_cfg_loader_find_nc(env->m_module->m_app, NULL);
            if (loader == NULL) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: popup_def %s: find fsm loader fail!", popup_name);
                return -1;
            }
                
            if (ui_sprite_ui_env_popup_def_load(env, popup_def, loader, cfg_value) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "ui_sprite_ui_env_load: popup_def %s: load runing fsm fail!",
                    popup_name);
                return -1;
            }
        }
    }

    return 0;
}

static int ui_sprite_ui_env_load_package_queues(ui_sprite_ui_env_t env, cfg_t cfg) {
    struct cfg_it queue_it;
    cfg_t queue_cfg;
    
    cfg_it_init(&queue_it, cfg);
    while((queue_cfg = cfg_it_next(&queue_it))) {
        const char * queue_name = cfg_get_string(queue_cfg, "name", NULL);
        const char * str_value;
        plugin_package_queue_policy_t policy = plugin_package_queue_policy_manual;
        plugin_ui_package_queue_managed_t managed_queue;
        
        if (queue_name == NULL) {
            CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load: package managed queues: queue name not configured");
            return -1;
        }

        if ((str_value  = cfg_get_string(queue_cfg, "policy", NULL))) {
            if (strcmp(str_value, "LRU") == 0) {
                policy = plugin_package_queue_policy_lru;
            }
            else if (strcmp(str_value, "MANUAL") == 0) {
                policy = plugin_package_queue_policy_manual;
            }
            else {
                CPE_ERROR(
                    env->m_module->m_em, "ui_sprite_ui_env_load: package managed queues: queue %s policy %s unknown",
                    queue_name, str_value);
                return -1;
            }
        }

        managed_queue = plugin_ui_package_queue_managed_create(env->m_env, queue_name, policy);
        if (managed_queue == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_load: package managed queues: queue %s create fail",
                queue_name);
            return -1;
        }
    }

    return 0;
}

int ui_sprite_ui_env_load_sound_cfg(ui_sprite_ui_env_t env, cfg_t cfg) {
    struct cfg_it cfg_it;
    cfg_t child_cfg;

    cfg_it_init(&cfg_it, cfg);
    while((child_cfg = cfg_it_next(&cfg_it))) {
        const char * group_name = cfg_name(child_cfg);
        ui_runtime_sound_group_t sound_group;
        const char * string_value;
        
        sound_group = ui_runtime_sound_group_find(env->m_module->m_runtime, group_name);
        if (sound_group == NULL) {
            sound_group = ui_runtime_sound_group_create(env->m_module->m_runtime, group_name, ui_runtime_sound_sfx);
            if (sound_group == NULL) {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load_sound_cfg: create sound group %s fail", group_name);
                return -1;
            }
        }

        if ((string_value = cfg_get_string(child_cfg, "schedule", NULL))) {
            if (strcmp(string_value, "queue") == 0) {
                ui_runtime_sound_group_set_schedule_type(sound_group, ui_runtime_sound_group_schedule_queue);
            }
            else {
                CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_load_sound_cfg: group %s unknown schedule %s", group_name, string_value);
                return -1;
            }
        }
    }
    
    return 0;
}

static void ui_sprite_ui_env_load_cfg_buttons(plugin_ui_cfg_button_t button_cfg, cfg_t part_cfg) {
    button_cfg->m_pos_adj.x = cfg_get_float(part_cfg, "pos-adj.x", 0.0f);
    button_cfg->m_pos_adj.y = cfg_get_float(part_cfg, "pos-adj.y", 0.0f);
    button_cfg->m_scale_adj.x = cfg_get_float(part_cfg, "scale-adj.x", 0.0f);
    button_cfg->m_scale_adj.y = cfg_get_float(part_cfg, "scale-adj.y", 0.0f);
    button_cfg->m_down_duration = cfg_get_float(part_cfg, "down-duration", 0.0f);
    cpe_str_dup(button_cfg->m_down_decorator, sizeof(button_cfg->m_down_decorator), cfg_get_string(part_cfg, "down-decorator", ""));
    button_cfg->m_raise_duration = cfg_get_float(part_cfg, "raise-duration", 0.0f);
    cpe_str_dup(button_cfg->m_raise_decorator, sizeof(button_cfg->m_raise_decorator), cfg_get_string(part_cfg, "raise-decorator", ""));
}

static int ui_sprite_ui_env_load_defaults(ui_sprite_ui_env_t env, cfg_t cfg) {
    int rv = 0;
    cfg_t part_cfg;

    if ((part_cfg = cfg_find_cfg(cfg, "button"))) {
        struct plugin_ui_cfg_button button_cfg;
        ui_sprite_ui_env_load_cfg_buttons(&button_cfg, part_cfg);
        plugin_ui_env_set_cfg_button(env->m_env, ui_control_type_button, &button_cfg);
    }
    
    if ((part_cfg = cfg_find_cfg(cfg, "toggle"))) {
        struct plugin_ui_cfg_button toggle_cfg;
        ui_sprite_ui_env_load_cfg_buttons(&toggle_cfg, part_cfg);
        plugin_ui_env_set_cfg_button(env->m_env, ui_control_type_toggle, &toggle_cfg);
    }
    
    return rv;
}
