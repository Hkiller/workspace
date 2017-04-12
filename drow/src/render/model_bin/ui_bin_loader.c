#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "ui_bin_loader_i.h"

static void ui_bin_loader_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_bin_loader = {
    "ui_bin_loader",
    ui_bin_loader_clear
};

ui_bin_loader_t
ui_bin_loader_create(gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em) {
    struct ui_bin_loader * loader;
    nm_node_t loader_node;

    assert(app);

    if (name == NULL) name = "ui_bin_loader";

    loader_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_bin_loader));
    if (loader_node == NULL) return NULL;

    loader = (ui_bin_loader_t)nm_node_data(loader_node);

    loader->m_app = app;
    loader->m_alloc = alloc;
    loader->m_em = em;
    loader->m_debug = 0;
    loader->m_root = NULL;

    nm_node_set_type(loader_node, &s_nm_node_type_ui_bin_loader);

    return loader;
}

static void ui_bin_loader_clear(nm_node_t node) {
    ui_bin_loader_t loader;

    loader = nm_node_data(node);

    if (loader->m_root) {
        mem_free(loader->m_alloc, loader->m_root);
    }
}

gd_app_context_t ui_bin_loader_app(ui_bin_loader_t loader) {
    return loader->m_app;
}

void ui_bin_loader_free(ui_bin_loader_t loader) {
    nm_node_t loader_node;
    assert(loader);

    loader_node = nm_node_from_data(loader);
    if (nm_node_type(loader_node) != &s_nm_node_type_ui_bin_loader) return;
    nm_node_free(loader_node);
}

ui_bin_loader_t
ui_bin_loader_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_bin_loader) return NULL;
    return (ui_bin_loader_t)nm_node_data(node);
}

ui_bin_loader_t
ui_bin_loader_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "ui_bin_loader";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_bin_loader) return NULL;
    return (ui_bin_loader_t)nm_node_data(node);
}

const char * ui_bin_loader_name(ui_bin_loader_t loader) {
    return nm_node_name(nm_node_from_data(loader));
}

const char * ui_bin_loader_root(ui_bin_loader_t loader) {
    return loader->m_root;
}

int ui_bin_loader_set_root(ui_bin_loader_t loader, const char * root) {
    if (loader->m_root) {
        mem_free(loader->m_alloc, loader->m_root);
    }

    if (root) {
        loader->m_root = cpe_str_mem_dup(loader->m_alloc, root);
        if (loader->m_root == NULL) return -1;
    }
    else {
        loader->m_root = NULL;
    }

    return 0;
}

extern int ui_data_bin_load_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
extern int ui_data_bin_load_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
extern int ui_data_bin_load_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
extern int ui_data_bin_load_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);

int ui_data_bin_loader_set_load_to_data_mgr(ui_bin_loader_t loader, ui_data_mgr_t mgr) {
    ui_data_mgr_set_loader(mgr, ui_data_src_type_module, ui_data_bin_load_module, loader);
    ui_data_mgr_set_loader(mgr, ui_data_src_type_sprite, ui_data_bin_load_sprite, loader);
    ui_data_mgr_set_loader(mgr, ui_data_src_type_action, ui_data_bin_load_action, loader);
    ui_data_mgr_set_loader(mgr, ui_data_src_type_layout, ui_data_bin_load_layout, loader);
    return 0;
}

extern int ui_data_bin_save_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
extern int ui_data_bin_save_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
extern int ui_data_bin_save_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
extern int ui_data_bin_save_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

int ui_data_bin_rm_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int ui_data_bin_rm_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int ui_data_bin_rm_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int ui_data_bin_rm_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

int ui_data_bin_loader_set_save_to_data_mgr(ui_bin_loader_t loader, ui_data_mgr_t mgr) {
    ui_data_mgr_set_saver(mgr, ui_data_src_type_module, ui_data_bin_save_module, ui_data_bin_rm_module, loader);
    ui_data_mgr_set_saver(mgr, ui_data_src_type_sprite, ui_data_bin_save_sprite, ui_data_bin_rm_sprite, loader);
    ui_data_mgr_set_saver(mgr, ui_data_src_type_action, ui_data_bin_save_action, ui_data_bin_rm_action, loader);
    ui_data_mgr_set_saver(mgr, ui_data_src_type_layout, ui_data_bin_save_layout, ui_data_bin_rm_layout, loader);
    return 0;
}

EXPORT_DIRECTIVE
int ui_bin_loader_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_bin_loader_t ui_bin_loader;

    ui_bin_loader =
        ui_bin_loader_create(
            app,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_bin_loader == NULL) return -1;

    ui_bin_loader->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_bin_loader_set_root(ui_bin_loader, cfg_get_string(cfg, "root", "")) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set root fail", ui_bin_loader_name(ui_bin_loader));
        ui_bin_loader_free(ui_bin_loader);
        return -1;
    }

    if (ui_bin_loader->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_bin_loader_name(ui_bin_loader));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_bin_loader_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_bin_loader_t ui_bin_loader;

    ui_bin_loader = ui_bin_loader_find_nc(app, gd_app_module_name(module));
    if (ui_bin_loader) {
        ui_bin_loader_free(ui_bin_loader);
    }
}
