#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "ui_ed_mgr_i.h"
#include "ui_ed_src_i.h"
#include "ui_ed_obj_i.h"
#include "ui_ed_obj_meta_i.h"

static void ui_ed_mgr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_ed_mgr = {
    "ui_ed_mgr",
    ui_ed_mgr_clear
};

ui_ed_mgr_t
ui_ed_mgr_create(gd_app_context_t app, ui_data_mgr_t data_mgr, mem_allocrator_t alloc, const char * name, error_monitor_t em) {
    ui_ed_mgr_t ed_mgr;
    nm_node_t ed_mgr_node;

    assert(app);

    if (name == NULL) name = "ui_ed_mgr";

    ed_mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_ed_mgr));
    if (ed_mgr_node == NULL) return NULL;

    ed_mgr = (ui_ed_mgr_t)nm_node_data(ed_mgr_node);
    ed_mgr->m_app = app;
    ed_mgr->m_alloc = alloc;
    ed_mgr->m_em = em;
    ed_mgr->m_data_mgr = data_mgr;

    mem_buffer_init(&ed_mgr->m_dump_buffer, alloc);

    if (cpe_hash_table_init(
            &ed_mgr->m_ed_srcs,
            alloc,
            (cpe_hash_fun_t) ui_ed_src_hash,
            (cpe_hash_eq_t) ui_ed_src_eq,
            CPE_HASH_OBJ2ENTRY(ui_ed_src, m_hh_for_mgr),
            -1) != 0)
    {
        mem_buffer_clear(&ed_mgr->m_dump_buffer);
        nm_node_free(ed_mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &ed_mgr->m_ed_objs,
            alloc,
            (cpe_hash_fun_t) ui_ed_obj_hash,
            (cpe_hash_eq_t) ui_ed_obj_eq,
            CPE_HASH_OBJ2ENTRY(ui_ed_obj, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&ed_mgr->m_ed_srcs);
        mem_buffer_clear(&ed_mgr->m_dump_buffer);
        nm_node_free(ed_mgr_node);
        return NULL;
    }

    ui_ed_obj_mgr_init_src_metas(ed_mgr);
    ui_ed_obj_mgr_init_metas(ed_mgr);

    nm_node_set_type(ed_mgr_node, &s_nm_node_type_ui_ed_mgr);

    return ed_mgr;
}

static void ui_ed_mgr_clear(nm_node_t node) {
    ui_ed_mgr_t ed_mgr = nm_node_data(node);

    ui_ed_src_free_all(ed_mgr);
    ui_ed_obj_mgr_fini_metas(ed_mgr);

    assert(cpe_hash_table_count(&ed_mgr->m_ed_srcs) == 0);
    cpe_hash_table_fini(&ed_mgr->m_ed_srcs);

    assert(cpe_hash_table_count(&ed_mgr->m_ed_objs) == 0);
    cpe_hash_table_fini(&ed_mgr->m_ed_objs);

    mem_buffer_clear(&ed_mgr->m_dump_buffer);
}

gd_app_context_t ui_ed_mgr_app(ui_ed_mgr_t mgr) {
    return mgr->m_app;
}

void ui_ed_mgr_free_src_by_type(ui_ed_mgr_t ed_mgr, ui_data_src_type_t src_type) {
    struct cpe_hash_it src_it;
    ui_ed_src_t src;

    cpe_hash_it_init(&src_it, &ed_mgr->m_ed_srcs);

    src = cpe_hash_it_next(&src_it);
    while (src) {
        ui_ed_src_t next = cpe_hash_it_next(&src_it);

        if (ui_data_src_type(src->m_data_src) == src_type) {
            ui_ed_src_free_i(src);
        }

        src = next;
    }
}

void ui_ed_mgr_free_obj_by_type(ui_ed_mgr_t ed_mgr, ui_ed_obj_type_t obj_type) {
    struct cpe_hash_it src_it;
    ui_ed_src_t src;

    cpe_hash_it_init(&src_it, &ed_mgr->m_ed_srcs);

    while((src = cpe_hash_it_next(&src_it))) {
        ui_ed_obj_free_childs_by_type_i(src->m_root_obj, obj_type);
    }
}

void ui_ed_mgr_free(ui_ed_mgr_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_ui_ed_mgr) return;
    nm_node_free(mgr_node);
}

ui_ed_mgr_t
ui_ed_mgr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_ed_mgr) return NULL;
    return (ui_ed_mgr_t)nm_node_data(node);
}

ui_ed_mgr_t
ui_ed_mgr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "ui_ed_mgr";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_ed_mgr) return NULL;
    return (ui_ed_mgr_t)nm_node_data(node);
}

const char * ui_ed_mgr_name(ui_ed_mgr_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

ui_data_mgr_t ui_ed_mgr_data_mgr(ui_ed_mgr_t ed_mgr) {
    return ed_mgr->m_data_mgr;
}

int ui_ed_mgr_register_src_type(ui_ed_mgr_t mgr, ui_data_src_type_t type, ui_ed_src_load_fun_t load_fun) {
    struct ui_ed_src_meta * src_meta;

    assert(load_fun);

    if ((uint8_t)type >= (uint8_t)UI_DATA_SRC_TYPE_MAX || (uint8_t)type < (uint8_t)UI_DATA_SRC_TYPE_MIN) {
        APP_CTX_ERROR(
            mgr->m_app, "%s: register type %d: type is unknown!", ui_ed_mgr_name(mgr), type);
        return -1;
    }

    src_meta = &mgr->m_src_metas[type - UI_DATA_SRC_TYPE_MIN];
    assert(src_meta);

    if (src_meta->m_load_fun != NULL) {
        APP_CTX_ERROR(
            mgr->m_app, "%s: register type %d: type is already registered!", ui_ed_mgr_name(mgr), type);
        return -1;
    }

    src_meta->m_load_fun = load_fun;

    return 0;
}

int ui_ed_mgr_unregister_src_type(ui_ed_mgr_t mgr, ui_data_src_type_t type) {
    struct ui_ed_src_meta * src_meta;

    if (type >= UI_DATA_SRC_TYPE_MAX || type < UI_DATA_SRC_TYPE_MIN) {
        APP_CTX_ERROR(
            mgr->m_app, "%s: unregister type %d: type is unknown!", ui_ed_mgr_name(mgr), type);
        return -1;
    }

    src_meta = &mgr->m_src_metas[type - UI_DATA_SRC_TYPE_MIN];
    assert(src_meta);

    if (src_meta->m_load_fun == NULL) {
        APP_CTX_ERROR(
            mgr->m_app, "%s: unregister type %d: type is not registered!", ui_ed_mgr_name(mgr), type);
        return -1;
    }

    ui_ed_mgr_free_src_by_type(mgr, type);

    bzero(src_meta, sizeof(*src_meta));

    return 0;
}

int ui_ed_mgr_save(ui_ed_mgr_t ed_mgr, const char * root, error_monitor_t em) {
    struct cpe_hash_it src_it;
    ui_ed_src_t src;
    int r = 0;

    cpe_hash_it_init(&src_it, &ed_mgr->m_ed_srcs);

    src = cpe_hash_it_next(&src_it);
    while (src) {
        ui_ed_src_t next = cpe_hash_it_next(&src_it);

        switch(src->m_state) {
        case ui_ed_src_state_normal:
            break;
        case ui_ed_src_state_new:
        case ui_ed_src_state_changed:
            if (ui_ed_src_save(src, root, em) != 0) r = -1;
            break;
        case ui_ed_src_state_removed: {
            ui_data_src_t data_src = src->m_data_src;
            ui_ed_src_free_i(src);
            ui_data_src_remove(data_src, root, em);
            break;
        }
        }

        src = next;
    }

    return r;
}

EXPORT_DIRECTIVE
int ui_ed_mgr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_ed_mgr_t ed_mgr;
    ui_data_mgr_t data_mgr;

    data_mgr = ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL));
    if (data_mgr == NULL) {
        APP_CTX_ERROR(app, "%s: create: data-mgr not exist", gd_app_module_name(module));
        return -1;
    }

    ed_mgr =
        ui_ed_mgr_create(
            app, data_mgr,
            gd_app_alloc(app), gd_app_module_name(module), gd_app_em(app));
    if (ed_mgr == NULL) return -1;

    ed_mgr->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ed_mgr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done", ui_ed_mgr_name(ed_mgr));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_ed_mgr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_ed_mgr_t ed_mgr;

    ed_mgr = ui_ed_mgr_find_nc(app, gd_app_module_name(module));
    if (ed_mgr) {
        ui_ed_mgr_free(ed_mgr);
    }
}
