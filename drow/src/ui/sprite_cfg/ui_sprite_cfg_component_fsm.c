#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_fsm.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_cfg_loader_i.h"

static int ui_sprite_cfg_do_load_fsm(ui_sprite_cfg_loader_t loader, ui_sprite_fsm_ins_t fsm, cfg_t cfg);

int ui_sprite_cfg_load_component_fsm(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_fsm_ins_t fsm = ui_sprite_component_data(component);
    ui_sprite_cfg_loader_t loader = ctx;

    if (ui_sprite_cfg_do_load_fsm(loader, fsm, cfg) != 0) {
        CPE_ERROR(loader->m_em, "%s: do load fsm fail!", ui_sprite_cfg_loader_name(loader));
        return -1;
    }

    return 0;
}

int ui_sprite_cfg_load_fsm_from_path(ui_sprite_cfg_loader_t loader, ui_sprite_fsm_ins_t fsm, const char * src_path) {
    cfg_t cfg = NULL;

    cfg = ui_sprite_cfg_loader_find_cfg(loader, src_path);
    if (cfg == NULL) {
        CPE_ERROR(loader->m_em, "%s: cfg %s not exist!", ui_sprite_cfg_loader_name(loader), src_path);
        return -1;
    }

    return ui_sprite_cfg_load_fsm(loader, fsm, cfg);
}

int ui_sprite_cfg_load_fsm(ui_sprite_cfg_loader_t loader, ui_sprite_fsm_ins_t fsm, cfg_t cfg) {
    if (ui_sprite_cfg_do_load_fsm(loader, fsm, cfg) != 0) {
        CPE_ERROR(loader->m_em, "%s: do load fsm fail!", ui_sprite_cfg_loader_name(loader));
        return -1;
    }

    return 0;
}

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_fsm(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * action_name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_fsm_ins_t fsm = (ui_sprite_fsm_ins_t)ui_sprite_fsm_action_fsm_create(fsm_state, action_name);
    const char * str_value;
    
    if (fsm == NULL) {
        CPE_ERROR(loader->m_em, "%s: create fsm action fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "fsm", NULL))) {
        if (ui_sprite_fsm_action_fsm_set_load_from((ui_sprite_fsm_action_fsm_t)fsm, str_value) != 0) {
            CPE_ERROR(loader->m_em, "%s: set fsm load-from %s fail!", ui_sprite_cfg_loader_name(loader), str_value);
            ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(fsm));
            return NULL;
        }
    }
    else {
        if (ui_sprite_cfg_do_load_fsm(loader, fsm, cfg_find_cfg(cfg, "fsm")) != 0) {
            CPE_ERROR(loader->m_em, "%s: do load fsm fail!", ui_sprite_cfg_loader_name(loader));
            ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(fsm));
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "data", NULL))) {
        dr_store_manage_t store_mgr;
        dr_store_t store;
        char lib_name[64];
        const char * meta_name;
        const char * sep;
        LPDRMETA meta;
        uint32_t data_capacity;
        
        sep = strchr(str_value, '.');
        if (sep == NULL) {
            CPE_ERROR(loader->m_em, "%s: set fsm data fail, %s format error!", ui_sprite_cfg_loader_name(loader), str_value);
            ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(fsm));
            return NULL;
        }
        cpe_str_dup_range(lib_name, sizeof(lib_name), str_value, sep);
        meta_name = sep + 1;
        
        store_mgr = dr_store_manage_find_nc(loader->m_app, NULL);
        if (store_mgr == NULL) {
            CPE_ERROR(loader->m_em, "%s: set fsm data fail, no dr_store_manager!", ui_sprite_cfg_loader_name(loader));
            ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(fsm));
            return NULL;
        }

        store = dr_store_find(store_mgr, lib_name);
        if (store == NULL) {
            CPE_ERROR(loader->m_em, "%s: set fsm data fail, metalib %s not exist!", ui_sprite_cfg_loader_name(loader), lib_name);
            ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(fsm));
            return NULL;
        }

        meta = dr_lib_find_meta_by_name(dr_store_lib(store), meta_name);
        if (meta == NULL) {
            CPE_ERROR(loader->m_em, "%s: set fsm data fail, meta %s not exist in metalib %s!", ui_sprite_cfg_loader_name(loader), meta_name, lib_name);
            ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(fsm));
            return NULL;
        }

        data_capacity = cfg_get_uint32(cfg, "data-capacity", 0);
        if (ui_sprite_fsm_action_fsm_init_data((ui_sprite_fsm_action_fsm_t)fsm, meta, (size_t)data_capacity) != 0) {
            CPE_ERROR(loader->m_em, "%s: set fsm data fail, meta=%s, capacity=%d!", ui_sprite_cfg_loader_name(loader), dr_meta_name(meta), data_capacity);
            ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(fsm));
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(fsm);
}

/*fsms*/
ui_sprite_fsm_ins_t
ui_sprite_cfg_loader_load_fsm_proto_from_path(ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, const char * name, const char * src_path) {
    cfg_t cfg = NULL;

    cfg = ui_sprite_cfg_loader_find_cfg(loader, src_path);
    if (cfg == NULL) {
        CPE_ERROR(loader->m_em, "%s: cfg %s not exist!", ui_sprite_cfg_loader_name(loader), src_path);
        return NULL;
    }

    return ui_sprite_cfg_loader_load_fsm_proto_from_cfg(loader, world, name, cfg);
}

ui_sprite_fsm_ins_t
ui_sprite_cfg_loader_load_fsm_proto_from_cfg(ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, const char * name, cfg_t cfg) {
    ui_sprite_fsm_ins_t proto_fsm;

    proto_fsm = ui_sprite_fsm_proto_create(world, name);
    if (proto_fsm == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: load fsm proto %s: create proto fsm fail!",
            ui_sprite_cfg_loader_name(loader), name);
        return NULL;
    }

    if (ui_sprite_cfg_do_load_fsm(loader, proto_fsm, cfg) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: load fsm proto %s: load fsm fail!",
            ui_sprite_cfg_loader_name(loader), name);
        ui_sprite_fsm_proto_free(proto_fsm);
        return NULL;
    }

    return proto_fsm;
}


static int ui_sprite_cfg_do_load_fsm_actions(ui_sprite_cfg_loader_t loader, ui_sprite_fsm_state_t fsm_state, cfg_t cfg) {
    struct cfg_it action_cfg_it;
    cfg_t child_cfg;

    cfg_it_init(&action_cfg_it, cfg);

    while((child_cfg = cfg_it_next(&action_cfg_it))) {
        const char * follow_to;
        ui_sprite_fsm_action_t fsm_action;
        const char * action_name;
        cfg_t action_cfg;

        action_cfg = cfg_child_only(child_cfg);
        if (action_cfg == NULL) {
            action_name = cfg_as_string(child_cfg, "");
            if (action_name == NULL) {
                CPE_ERROR(
                    loader->m_em, "%s: do load fsm: state %s: action format error!",
                    ui_sprite_cfg_loader_name(loader), ui_sprite_fsm_state_name(fsm_state));
                return -1;
            }
        }
        else {
            action_name = cfg_name(action_cfg);
        }

        fsm_action = ui_sprite_cfg_loader_load_action_from_cfg(loader, fsm_state, action_name, action_cfg);
        if (fsm_action == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: do load fsm: state %s: create action fail!",
                ui_sprite_cfg_loader_name(loader), ui_sprite_fsm_state_name(fsm_state));
            return -1;
        }

        if ((follow_to = cfg_get_string(action_cfg, "follow", NULL))) {
            if (ui_sprite_fsm_action_set_follow_to(fsm_action, follow_to) != 0) {
                CPE_ERROR(
                    loader->m_em, "%s: do load fsm: state %s: set follow to %s fail!",
                    ui_sprite_cfg_loader_name(loader), ui_sprite_fsm_state_name(fsm_state), follow_to);
                return -1;
            }
        }
    }

    return 0;
}

static int ui_sprite_cfg_do_load_fsm_transitions(ui_sprite_cfg_loader_t loader, ui_sprite_fsm_state_t fsm_state, cfg_t cfg) {
    struct cfg_it transition_cfg_it;
    cfg_t transition_cfg;

    cfg_it_init(&transition_cfg_it, cfg);

    while((transition_cfg = cfg_it_next(&transition_cfg_it))) {
        ui_sprite_fsm_transition_t fsm_transition;
        const char * event;
        const char * to_state;
        const char * call_state;

        if ((event = cfg_get_string(transition_cfg, "event", NULL)) == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: do load fsm: state %s: transaction 'event' not configured fail!",
                ui_sprite_cfg_loader_name(loader), ui_sprite_fsm_state_name(fsm_state));
            return -1;
        }

        to_state = cfg_get_string(transition_cfg, "switch-to", NULL);
        call_state = cfg_get_string(transition_cfg, "call", NULL);

        if (to_state == NULL && call_state == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: do load fsm: state %s: transaction 'switch-to' or 'call' not configured fail!",
                ui_sprite_cfg_loader_name(loader), ui_sprite_fsm_state_name(fsm_state));
            return -1;
        }

        fsm_transition = 
            ui_sprite_fsm_transition_create(
                fsm_state, event, to_state, call_state, cfg_get_string(transition_cfg, "condition", NULL));
        if (fsm_transition == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: do load fsm: state %s: crate transition fail!",
                ui_sprite_cfg_loader_name(loader), ui_sprite_fsm_state_name(fsm_state));
            return -1;
        }
    }

    return 0;
}

static int ui_sprite_cfg_do_load_fsm(ui_sprite_cfg_loader_t loader, ui_sprite_fsm_ins_t fsm, cfg_t cfg) {
    const char * default_state = NULL;
    const char * default_call_state = NULL;
    struct cfg_it child_cfg_it;
    cfg_t child_cfg;

    if ((child_cfg = cfg_find_cfg(cfg, "import"))) {
        const char * import_path = cfg_as_string(child_cfg, NULL);
        cfg_t import_cfg;

        if (import_path == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: do load fsm: import format error!",
                ui_sprite_cfg_loader_name(loader));
            return -1;
        }

        import_cfg = ui_sprite_cfg_loader_find_cfg(loader, import_path);
        if (import_cfg == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: do load fsm: import from %s: import cfg not exist!",
                ui_sprite_cfg_loader_name(loader), import_path);
            return -1;
        }

        if (ui_sprite_cfg_do_load_fsm(loader, fsm, import_cfg) != 0){
            CPE_ERROR(
                loader->m_em, "%s: do load fsm: import from %s: load fail!",
                ui_sprite_cfg_loader_name(loader), import_path);
            return -1;
        }
    }
    
    cfg_it_init(&child_cfg_it, cfg);

    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        const char * name = cfg_name(child_cfg);
        if (strcmp(name, "import") == 0) {
            continue;
        }
        else if (strcmp(name, "init-state") == 0) {
            default_state = cfg_as_string(child_cfg, NULL);
        }
        else if (strcmp(name, "init-call-state") == 0) {
            default_call_state = cfg_as_string(child_cfg, NULL);
        }
        else {
            ui_sprite_fsm_state_t fsm_state = ui_sprite_fsm_state_find_by_name(fsm, name);
            if (fsm_state == NULL) {
                fsm_state = ui_sprite_fsm_state_create(fsm, name);
                if (fsm_state == NULL) {
                    CPE_ERROR(
                        loader->m_em, "%s: do load fsm: create state %s fail!",
                        ui_sprite_cfg_loader_name(loader), name);
                    return -1;
                }
            }

            if (ui_sprite_cfg_do_load_fsm_actions(loader, fsm_state, cfg_find_cfg(child_cfg, "Actions")) != 0) return -1;
            if (ui_sprite_cfg_do_load_fsm_transitions(loader, fsm_state, cfg_find_cfg(child_cfg, "Transitions")) != 0) return -1;
        }
    }

    if (default_state) {
        if (ui_sprite_fsm_set_default_state(fsm, default_state) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: do load fsm: set default state %s fail!",
                ui_sprite_cfg_loader_name(loader), default_state);
            return -1;
        }
    }

    if (default_call_state) {
        if (ui_sprite_fsm_set_default_call_state(fsm, default_call_state) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: do load fsm: set default call state %s fail!",
                ui_sprite_cfg_loader_name(loader), default_call_state);
            return -1;
        }
    }

    return 0;
}
