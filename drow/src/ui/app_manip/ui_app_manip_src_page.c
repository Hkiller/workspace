#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "ui_app_manip_src_i.h"

int ui_app_manip_collect_src_from_control_attr(ui_data_src_group_t src_group, ui_cache_group_t cache_group, const char * name, const char * value, error_monitor_t em) {
    char buf[256];
    
    if (value[0] == '"') {
        const char * e;
        value++;
        e = strrchr(value, '"');
        if (e) {
            cpe_str_dup_range(buf, sizeof(buf), value, e);
            value = buf;
        }
    }
    
    if (strcmp(name, "back-res") == 0 || strcmp(name, "down-res") == 0) {
        if (ui_app_manip_collect_src_by_res(src_group, cache_group, value, em) != 0) {
            CPE_ERROR(
                em, "ui_app_manip_collect_src_from_control_attr %s: ==> %s fail",
                name, value);
            return -1;
        }
    }
    else if (strcmp(name, "text-font-family") == 0) {
        if (cpe_str_start_with(value, "sys::")) {
            char value_path_buf[128];
            snprintf(value_path_buf, sizeof(value_path_buf), "sysfont/sysFont_%d.ttf", atoi(value + 5));
                    
            if (ui_data_src_group_add_src_by_res(src_group, value_path_buf) != 0) {
                CPE_ERROR(
                    em, "ui_app_manip_collect_src_from_control_attr %s: ==> %s fail",
                    name, value_path_buf);
                return -1;
            }
        }
        else if (cpe_str_start_with(value, "art::")) {
            char value_path_buf[128];
            snprintf(value_path_buf, sizeof(value_path_buf), "ArtFont/artFont_%d.ibk", atoi(value + 5));

            if (ui_data_src_group_add_src_by_res(src_group, value_path_buf) != 0) {
                CPE_ERROR(
                    em, "ui_app_manip_collect_src_from_control_attr %s: ==> %s fail",
                    name, value_path_buf);
                return -1;
            }
        }
        else {
            CPE_ERROR(em, "ui_app_manip_collect_src_from_control_attr %s: family %s unknown", name, value);
            return -1;
        }
    }

    return 0;
}

int ui_app_manip_collect_src_from_control_binding(ui_data_src_group_t src_group, ui_cache_group_t cache_group, const char * name, cfg_t control_attr_cfg, error_monitor_t em) {
    if (cfg_type(control_attr_cfg) == CPE_CFG_TYPE_STRING) {
        const char * res = cfg_as_string(control_attr_cfg, NULL);
        if (res && ui_app_manip_collect_src_from_control_attr(src_group, cache_group, name, res, em) != 0) return -1;
    }
    else if (cfg_type(control_attr_cfg) == CPE_CFG_TYPE_SEQUENCE) {
        struct cfg_it child_it;
        cfg_t child_cfg;
        int rv = 0;
        cfg_it_init(&child_it, control_attr_cfg);
        while((child_cfg = cfg_it_next(&child_it))) {
            const char * res = cfg_as_string(child_cfg, NULL);
            if (res && ui_app_manip_collect_src_from_control_attr(src_group, cache_group, name, res, em) != 0) {
                rv = -1;
            }
        }

        return rv;
    }
    
    return 0;
}

int ui_app_manip_collect_src_from_page(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t page_cfg, error_monitor_t em) {
    const char * page_name = cfg_name(page_cfg);
    struct cfg_it cfg_binding_it;
    cfg_t cfg_binding;
    struct cfg_it cfg_anim_it;
    cfg_t cfg_anim;
    int rv = 0;

    /*main res*/
    do {
        const char * page_res_path;
        char res_path_buf[128];
        char * p;
    
        page_res_path = cfg_get_string(page_cfg, "load-from", NULL);
        if (page_res_path == NULL) break;
        
        cpe_str_dup(res_path_buf, sizeof(res_path_buf), page_res_path);
        p = strstr(res_path_buf, ".bin");
        if (p) *p = 0;

        if (ui_data_src_group_add_src_by_res(src_group, res_path_buf) != 0) {
            CPE_ERROR(em, "ui_app_manip_collect_src_from_page: page %s add src %s fail", page_name, res_path_buf);
            rv = -1;
            break;
        }
    } while(0);

    /*bindings*/
    cfg_it_init(&cfg_binding_it, cfg_find_cfg(page_cfg, "bindings"));
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

    /*animations*/
    cfg_it_init(&cfg_anim_it, cfg_find_cfg(page_cfg, "animations"));
    while((cfg_anim = cfg_it_next(&cfg_anim_it))) {
        const char * res;
                
        cfg_anim = cfg_child_only(cfg_anim);
        if (cfg_anim == NULL) continue;

        if ((res = cfg_as_string(cfg_anim, NULL))) {
            if (ui_app_manip_collect_src_by_res(src_group, cache_group, res, em) != 0) {
                rv = -1;
            }
        }
    }
            
    if (ui_app_manip_collect_src_from_additions(src_group, cache_group, cfg_find_cfg(page_cfg, "addition-res"), em) != 0) {
        CPE_ERROR(em, "ui_app_manip_collect_src_from_page: collect src from addition error");
        rv = -1;
    }

    return rv;
}

int ui_app_manip_collect_src_from_popup(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t popup_cfg, error_monitor_t em) {
    struct cfg_it cfg_binding_it;
    cfg_t cfg_binding;
    const char * popup_name = cfg_name(popup_cfg);
    const char * popup_res_path;
    char * p;
    int rv = 0;
    
    popup_res_path = cfg_get_string(popup_cfg, "load-from", NULL);
    if (popup_res_path) {
        char res_path_buf[128];
        
        cpe_str_dup(res_path_buf, sizeof(res_path_buf), popup_res_path);
        p = strstr(res_path_buf, ".bin");
        if (p) *p = 0;

        if (ui_app_manip_collect_src_by_res(src_group, cache_group, res_path_buf, em) != 0) {
            CPE_ERROR(em, "convert_pack_phase_from_popups: popup %s use res %s not exist", popup_name, res_path_buf);
            rv = -1;
        }
    }
        
    cfg_it_init(&cfg_binding_it, cfg_find_cfg(popup_cfg, "bindings"));
    while((cfg_binding = cfg_it_next(&cfg_binding_it))) {
        struct cfg_it cfg_attr_it;
        cfg_t cfg_attr;

        cfg_it_init(&cfg_attr_it, cfg_binding);
        while((cfg_attr = cfg_it_next(&cfg_attr_it))) {
            const char * name = cfg_name(cfg_attr);

            if (strcmp(name, "back-res") == 0 || strcmp(name, "down-res") == 0 ) {
                const char * res = cfg_as_string(cfg_attr, NULL);

                if (ui_app_manip_collect_src_by_res(src_group, cache_group, res, em) != 0) {
                    CPE_ERROR(em, "popup %s: add src %s fail", popup_name, res);
                    rv = -1;
                }
            }
        }
    }

    if (ui_app_manip_collect_src_from_fsm(src_group, cache_group, cfg_find_cfg(popup_cfg, "runing-fsm"), em) != 0) {
        CPE_ERROR(em, "popup %s: collect src from runing-fsm error", popup_name);
        rv = -1;
    }
            
    if (ui_app_manip_collect_src_from_additions(src_group, cache_group, cfg_find_cfg(popup_cfg, "addition-res"), em) != 0) {
        CPE_ERROR(em, "popup %s: collect src from addition error", popup_name);
        rv = -1;
    }

    return rv;
}
