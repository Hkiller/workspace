#include "cpe/utils/string_utils.h"
#include "ui_app_manip_src_i.h"

static int ui_app_manip_collect_src_from_fsm_action_ui_play_anim(
    ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em)
{
    int rv = 0;

    if (ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "back_res", NULL), em) != 0
        || ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "back-res", NULL), em) != 0
        || ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "down_res", NULL), em) != 0
        || ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "down-res", NULL), em) != 0
        || ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "disable_res", NULL), em) != 0
        || ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "disable-res", NULL), em) != 0
        )
    {
        rv = -1;
    }

    return rv;
}

static int ui_app_manip_collect_src_from_fsm_action_ui_set_values(
    ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em)
{
    struct cfg_it cfg_value_it;
    cfg_t cfg_value;
    int rv = 0;
    
    /*values*/
    cfg_it_init(&cfg_value_it, cfg_find_cfg(action_cfg, "values"));
    while((cfg_value = cfg_it_next(&cfg_value_it))) {
        const char * value = cfg_get_string(cfg_value, "value", NULL);
        const char * name = cfg_get_string(cfg_value, "name", NULL);
        if (name && value) {
            if (ui_app_manip_collect_src_from_control_attr(src_group, cache_group, name, value, em) != 0) {
                rv = -1;
                continue;
            }
        }
    }

    return rv;
}

static int ui_app_manip_collect_src_from_fsm_action_render_with_obj(
    ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em)
{
    int rv = 0;

    if (ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "obj-res", NULL), em) != 0) {
        rv = -1;
    }
    
    return rv;
}

int ui_app_manip_collect_src_from_fsm_action_chipmunk_with_collision(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    struct cfg_it childs;
    cfg_t child_cfg;
    int rv = 0;
    
    cfg_it_init(&childs, cfg_find_cfg(action_cfg, "bodies"));
    while((child_cfg = cfg_it_next(&childs))) {
        if (ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(child_cfg, "load-from", NULL), em) != 0) rv = -1;
    }

    return rv;
}

int ui_app_manip_collect_src_from_fsm_action_show_animation(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    int rv = 0;

    if (ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "anim-res", NULL), em) != 0) rv = -1;

    return rv;
}

int ui_app_manip_collect_src_from_fsm_action_show_template(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    struct cfg_it cfg_binding_it;
    cfg_t cfg_binding;
    int rv = 0;

    if (ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "res", NULL), em) != 0) rv = -1;

    /*bindings*/
    cfg_it_init(&cfg_binding_it, cfg_find_cfg(action_cfg, "bindings"));
    while((cfg_binding = cfg_it_next(&cfg_binding_it))) {
        struct cfg_it cfg_attr_it;
        cfg_t cfg_attr;

        cfg_it_init(&cfg_attr_it, cfg_binding);
        while((cfg_attr = cfg_it_next(&cfg_attr_it))) {
            const char * name = cfg_name(cfg_attr);
            if (ui_app_manip_collect_src_from_control_binding(src_group, cache_group, name, cfg_attr, em)) {
                rv = -1;
                continue;
            }
        }
    }
    
    return rv;
}

int ui_app_manip_collect_src_from_fsm_action_show_page(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    cfg_t r = action_cfg;
    cfg_t p;
    cfg_t page_cfg;
    const char * page_name;
    
    while((p = cfg_parent(r))) {
        r = p;
    }

    page_name = cfg_get_string(action_cfg, "page", NULL);
    if (page_name == NULL) {
        CPE_ERROR(em, "action ui-show-page: page not configured!");
        return -1;
    }
    
    page_cfg = cfg_find_cfg(cfg_find_cfg(r, "ui.pages"), page_name);
    if (page_cfg == NULL) {
        CPE_ERROR(em, "page %s not exist!", page_name);
        return -1;
    }
    
    return ui_app_manip_collect_src_from_page(src_group, cache_group, page_cfg, em);
}

int ui_app_manip_collect_src_from_fsm_action_show_popup(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    cfg_t r = action_cfg;
    cfg_t p;
    cfg_t popup_cfg;
    const char * popup_name;
    
    while((p = cfg_parent(r))) {
        r = p;
    }

    popup_name = cfg_get_string(action_cfg, "popup", NULL);
    if (popup_name == NULL) {
        CPE_ERROR(em, "action ui-show-popup: popup not configured!");
        return -1;
    }
    
    popup_cfg = cfg_find_cfg(cfg_find_cfg(r, "ui.popups"), popup_name);
    if (popup_cfg == NULL) {
        CPE_ERROR(em, "popup %s not exist!", popup_name);
        return -1;
    }
    
    return ui_app_manip_collect_src_from_popup(src_group, cache_group, popup_cfg, em);
}

int ui_app_manip_collect_src_from_fsm_action_move_by_plan(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    int rv = 0;
    const char * moving_res = cfg_get_string(action_cfg, "res", NULL);

    if (moving_res && strchr(moving_res, '@') == NULL) {
        if (ui_data_src_group_add_src_by_path(src_group, moving_res, ui_data_src_type_moving_plan) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int ui_app_manip_collect_src_from_fsm_action_scrollmap_gen_team(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    int rv = 0;
    const char * moving_res = cfg_get_string(action_cfg, "res", NULL);

    if (moving_res && strchr(moving_res, '@') == NULL) {
        if (ui_data_src_group_add_src_by_path(src_group, moving_res, ui_data_src_type_moving_plan) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int ui_app_manip_collect_src_from_fsm_action_spine_ui_resize(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    int rv = 0;
    const char * spine_res = cfg_get_string(action_cfg, "res", NULL);

    if (spine_res) {
        if (ui_app_manip_collect_src_by_res(src_group, cache_group, spine_res, em) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int ui_app_manip_collect_src_from_fsm_action_sound(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    int rv = 0;
    const char * audio_res = cfg_get_string(action_cfg, "res", NULL);

    if (audio_res && strchr(audio_res, '@') == NULL) {
        if (ui_cache_group_add_res_by_path(cache_group, audio_res) != 0) rv = -1;
    }

    return rv;
}

int ui_app_manip_collect_src_from_fsm_action(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em) {
    const char * action_type = cfg_name(action_cfg);

    if (strcmp(action_type, "run-fsm") == 0) {
        return ui_app_manip_collect_src_from_fsm(src_group, cache_group, cfg_find_cfg(action_cfg, "fsm"), em);
    }
    else if (strcmp(action_type, "ui-play-anim") == 0) {
        return ui_app_manip_collect_src_from_fsm_action_ui_play_anim(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "ui-set-value") == 0 || strcmp(action_type, "ui-bind-value") == 0) {
        return ui_app_manip_collect_src_from_fsm_action_ui_set_values(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "render-with-obj") == 0) {
        return ui_app_manip_collect_src_from_fsm_action_render_with_obj(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "chipmunk-with-collision") == 0) {
        return ui_app_manip_collect_src_from_fsm_action_chipmunk_with_collision(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "show-animation") == 0) {
        return ui_app_manip_collect_src_from_fsm_action_show_animation(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "show-template") == 0) {
        return ui_app_manip_collect_src_from_fsm_action_show_template(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "ui-show-page") == 0) {
        return ui_app_manip_collect_src_from_fsm_action_show_page(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "ui-show-popup") == 0) {
        return ui_app_manip_collect_src_from_fsm_action_show_popup(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "move-by-plan") == 0
             || strcmp(action_type, "flight-move-by-plan") == 0)
    {
        return ui_app_manip_collect_src_from_fsm_action_move_by_plan(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "spine-move-entity") == 0) {
        return ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(action_cfg, "res", NULL), em);
    }
    else if (strcmp(action_type, "scrollmap-gen-team") == 0) {
        return ui_app_manip_collect_src_from_fsm_action_scrollmap_gen_team(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "spine-ui-resize-follow") == 0
             || strcmp(action_type, "spine-ui-resize-to") == 0)
    {
        return ui_app_manip_collect_src_from_fsm_action_spine_ui_resize(src_group, cache_group, action_cfg, em);
    }
    else if (strcmp(action_type, "play-sfx") == 0
             || strcmp(action_type, "play-bgm") == 0
             || strcmp(action_type, "trigger-sfx") == 0)
    {
        return ui_app_manip_collect_src_from_fsm_action_sound(src_group, cache_group, action_cfg, em);
    }
    else {
        //printf("xxxxxx: %s\n", action_type);
    }
    
    return 0;
}

