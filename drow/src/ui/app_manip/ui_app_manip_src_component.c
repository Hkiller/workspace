#include "cpe/utils/string_utils.h"
#include "render/model/ui_object_ref.h"
#include "ui_app_manip_src_i.h"

int ui_app_manip_collect_src_from_entity(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t entity_cfg, error_monitor_t em) {
    const char * import;
    int rv = 0;
    struct cfg_it addition_res_it;
    cfg_t addition_res;
    
    import = cfg_get_string(entity_cfg, "import", NULL);
    if(import) {
        cfg_t import_cfg;
        cfg_t root = entity_cfg;
        
        while(cfg_parent(root)) root = cfg_parent(root);

        import_cfg = cfg_find_cfg(root, import);
        if (import_cfg == NULL) {
            CPE_ERROR(em, "ui_app_manip_collect_src_from_entity: import entity %s not exist!", import);
            rv = -1;
        }
        else if (ui_app_manip_collect_src_from_entity(src_group, cache_group, import_cfg, em) != 0) {
            rv = -1;
        }
    }

    cfg_it_init(&addition_res_it, cfg_find_cfg(entity_cfg, "addition-res"));
    while((addition_res = cfg_it_next(&addition_res_it))) {
        const char * full_path = cfg_as_string(addition_res, NULL);
        if (full_path == NULL) {
            CPE_ERROR(em, "ui_app_manip_collect_src_from_entity: addition res format error!");
            rv = -1;
        }
        else if (ui_app_manip_collect_src_by_res(src_group, cache_group, full_path, em) != 0) {
            rv = -1;
        }
    }
    
    if (ui_app_manip_collect_src_from_components(src_group, cache_group, cfg_find_cfg(entity_cfg, "components"), em) != 0) {
        rv = -1;
    }

    return rv;
}

int ui_app_manip_collect_src_from_components(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t components_cfg, error_monitor_t em) {
    struct cfg_it childs;
    cfg_t child_cfg;
    int rv = 0;
    
    cfg_it_init(&childs, components_cfg);

    while((child_cfg = cfg_it_next(&childs))) {
        child_cfg = cfg_child_only(child_cfg);
        if (child_cfg == NULL) continue;
        
        if (ui_app_manip_collect_src_from_component(src_group, cache_group, child_cfg, em) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int ui_app_manip_collect_src_from_component_animation(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t component_cfg, error_monitor_t em) {
    struct cfg_it childs;
    cfg_t child_cfg;
    int rv = 0;
    
    cfg_it_init(&childs, cfg_find_cfg(component_cfg, "resources"));
    while((child_cfg = cfg_it_next(&childs))) {
        if (ui_app_manip_collect_src_by_res(src_group, cache_group, cfg_get_string(child_cfg, "res", NULL), em) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int ui_app_manip_collect_src_from_component_barrage_obj(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t component_cfg, error_monitor_t em) {
    cfg_t cfg = cfg_find_cfg(component_cfg, "barrages");
    struct cfg_it emitter_cfgs;
    cfg_t emitter_cfg;
    int rv = 0;

	if(cfg_type(cfg) == CPE_CFG_TYPE_STRING) {
        cfg_t root_cfg = cfg;
        cfg_t data_cfg;
        
        while(cfg_parent(root_cfg)) root_cfg = cfg_parent(root_cfg);

        data_cfg = cfg_find_cfg(root_cfg, cfg_as_string(cfg, NULL));
        if (data_cfg == NULL) {
            CPE_ERROR(em, "ui_app_manip_collect_src_from_entity: barrages %s not exist!", cfg_as_string(cfg, NULL));
            return -1;
        }
        cfg_it_init(&emitter_cfgs, data_cfg);
    }
    else {
        cfg_it_init(&emitter_cfgs, cfg);
    }

    while((emitter_cfg = cfg_it_next(&emitter_cfgs))) {
        const char * path = cfg_get_string(emitter_cfg, "use", NULL);
        if (path == NULL) {
            CPE_ERROR(em, "ui_app_manip_collect_src_from_component_barrage_obj: barrages format error, use not config!");
            return -1;
        }

        if (ui_data_src_group_add_src_by_path(src_group, path, ui_data_src_type_barrage) != 0) rv = -1;
    }
    
    return rv;
}

int ui_app_manip_collect_src_from_component_chipmunk_obj(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t component_cfg, error_monitor_t em) {
    struct cfg_it childs;
    cfg_t child_cfg;
    int rv = 0;
    char buf[64];
    char * p;
    
    cfg_it_init(&childs, cfg_find_cfg(component_cfg, "bodies"));
    while((child_cfg = cfg_it_next(&childs))) {
        const char * load_from = cfg_get_string(child_cfg, "load-from", NULL);
        if (load_from == NULL) continue;

        cpe_str_dup(buf, sizeof(buf), load_from);
        p = strrchr(buf, '#');
        if (p) *p = 0;
        
        if (ui_data_src_group_add_src_by_path(src_group, buf, ui_data_src_type_chipmunk_scene) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int ui_app_manip_collect_src_from_component(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t component_cfg, error_monitor_t em) {
    const char * component_type;

    component_type = cfg_name(component_cfg);

    if (strcmp(component_type, "Animation") == 0) {
        return ui_app_manip_collect_src_from_component_animation(src_group, cache_group, component_cfg, em);
    }
    else if (strcmp(component_type, "BarrageObj") == 0) {
        return ui_app_manip_collect_src_from_component_barrage_obj(src_group, cache_group, component_cfg, em);
    }
    else if (strcmp(component_type, "ChipmunkObj") == 0) {
        return ui_app_manip_collect_src_from_component_chipmunk_obj(src_group, cache_group, component_cfg, em);
    }
    else if (strcmp(component_type, "Fsm") == 0) {
        return ui_app_manip_collect_src_from_fsm(src_group, cache_group, component_cfg, em);
    }
    else {
        return 0;
    }
}

int ui_app_manip_collect_src_from_fsm_state(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t state_cfg, error_monitor_t em) {
    struct cfg_it childs;
    cfg_t child_cfg;
    int rv = 0;
    
    cfg_it_init(&childs, cfg_find_cfg(state_cfg, "Actions"));

    while((child_cfg = cfg_it_next(&childs))) {
        if (ui_app_manip_collect_src_from_fsm_action(src_group, cache_group, cfg_child_only(child_cfg), em) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int ui_app_manip_collect_src_from_fsm(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t fsm_cfg, error_monitor_t em) {
    struct cfg_it childs;
    cfg_t child_cfg;
    const char * import;
    int rv = 0;

    import = cfg_get_string(fsm_cfg, "import", NULL);
    if(import) {
        cfg_t import_cfg;
        cfg_t root = fsm_cfg;
        
        while(cfg_parent(root)) root = cfg_parent(root);

        import_cfg = cfg_find_cfg(root, import);
        if (import_cfg == NULL) {
            CPE_ERROR(em, "ui_app_manip_collect_src_from_fsm: import fsm %s not exist!", import);
            rv = -1;
        }
        else {
            if (ui_app_manip_collect_src_from_fsm(src_group, cache_group, import_cfg, em) != 0) {
                rv = -1;
            }
        }
    }
    
    cfg_it_init(&childs, fsm_cfg);
    while((child_cfg = cfg_it_next(&childs))) {
        const char * name = cfg_name(child_cfg);
        
        if (strcmp(name, "init-state") == 0) continue;
        if (strcmp(name, "init-call-state") == 0) continue;
        if (strcmp(name, "import") == 0) continue;

        if (ui_app_manip_collect_src_from_fsm_state(src_group, cache_group, child_cfg, em) != 0) {
            rv = -1;
        }
    }

    return rv;
    
}

int ui_app_manip_collect_src_by_res(ui_data_src_group_t src_group, ui_cache_group_t cache_group, const char * res, error_monitor_t em) {
    if (res == NULL) return 0;
    if (res[0] == '@' || res[0] == ':') return 0;

    if (cpe_str_start_with(res, "tiledmap-layer:")) {
        return 0;
    }

    return ui_data_src_group_add_src_by_res(src_group, res);
}
