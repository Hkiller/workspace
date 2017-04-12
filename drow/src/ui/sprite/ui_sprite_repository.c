#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "ui_sprite_repository_i.h"
#include "ui_sprite_world_i.h"
#include "ui_sprite_component_meta_i.h"
#include "ui_sprite_event_meta_i.h"

static void ui_sprite_repository_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_repository = {
    "ui_sprite_repository",
    ui_sprite_repository_clear
};

ui_sprite_repository_t
ui_sprite_repository_create(gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em) {
    struct ui_sprite_repository * repo;
    nm_node_t repo_node;

    assert(app);

    if (name == NULL) name = "ui_sprite_repository";

    repo_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_repository));
    if (repo_node == NULL) return NULL;

    repo = (ui_sprite_repository_t)nm_node_data(repo_node);

    repo->m_app = app;
    repo->m_alloc = alloc;
    repo->m_em = em;
    repo->m_debug = 0;

    TAILQ_INIT(&repo->m_worlds);

    repo->m_computer = xcomputer_create(alloc, em);
    if (repo->m_computer == NULL) {
        nm_node_free(repo_node);
        return NULL;
    }

    if (ui_sprite_repository_register_functions(repo) != 0) {
        xcomputer_free(repo->m_computer);
        nm_node_free(repo_node);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &repo->m_component_metas,
            alloc,
            (cpe_hash_fun_t) ui_sprite_component_meta_hash,
            (cpe_hash_eq_t) ui_sprite_component_meta_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_component_meta, m_hh_for_repo),
            -1) != 0)
    {
        xcomputer_free(repo->m_computer);
        nm_node_free(repo_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &repo->m_event_metas,
            alloc,
            (cpe_hash_fun_t) ui_sprite_event_meta_hash,
            (cpe_hash_eq_t) ui_sprite_event_meta_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_event_meta, m_hh_for_repo),
            -1) != 0)
    {
        cpe_hash_table_fini(&repo->m_component_metas);
        xcomputer_free(repo->m_computer);
        nm_node_free(repo_node);
        return NULL;
    }

    mem_buffer_init(&repo->m_dump_buffer, alloc);

    nm_node_set_type(repo_node, &s_nm_node_type_ui_sprite_repository);

    return repo;
}

static void ui_sprite_repository_clear(nm_node_t node) {
    ui_sprite_repository_t repo;
    repo = (ui_sprite_repository_t)nm_node_data(node);

    while(!TAILQ_EMPTY(&repo->m_worlds)) {
        ui_sprite_world_free(TAILQ_FIRST(&repo->m_worlds));
    }

    ui_sprite_component_meta_free_all(repo);
    cpe_hash_table_fini(&repo->m_component_metas);

    ui_sprite_event_meta_free_all(repo);
    cpe_hash_table_fini(&repo->m_event_metas);

    xcomputer_free(repo->m_computer);

    mem_buffer_clear(&repo->m_dump_buffer);
}

gd_app_context_t ui_sprite_repository_app(ui_sprite_repository_t repo) {
    return repo->m_app;
}

void ui_sprite_repository_free(ui_sprite_repository_t repo) {
    nm_node_t repo_node;
    assert(repo);

    repo_node = nm_node_from_data(repo);
    if (nm_node_type(repo_node) != &s_nm_node_type_ui_sprite_repository) return;
    nm_node_free(repo_node);
}

ui_sprite_repository_t
ui_sprite_repository_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_repository) return NULL;
    return (ui_sprite_repository_t)nm_node_data(node);
}

ui_sprite_repository_t
ui_sprite_repository_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "ui_sprite_repository";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_repository) return NULL;
    return (ui_sprite_repository_t)nm_node_data(node);
}

const char * ui_sprite_repository_name(ui_sprite_repository_t repo) {
    return nm_node_name(nm_node_from_data(repo));
}

xcomputer_t ui_sprite_repository_computer(ui_sprite_repository_t repo) {
    return repo->m_computer;
}

EXPORT_DIRECTIVE
int ui_sprite_repository_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_repository_t ui_sprite_repository;
    struct cfg_it child_it;
    cfg_t child_cfg;

    ui_sprite_repository =
        ui_sprite_repository_create(
            app,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_repository == NULL) return -1;

    ui_sprite_repository->m_debug = cfg_get_int32(cfg, "debug", 0);

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "event-debug-level"));
    while((child_cfg = cfg_it_next(&child_it))) {
        if (ui_sprite_repository_register_event_debug_level(
                ui_sprite_repository, cfg_name(child_cfg), cfg_as_uint8(cfg, 2))
            != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: set event %s debug level fail!",
                ui_sprite_repository_name(ui_sprite_repository), 
                cfg_name(cfg));
            ui_sprite_repository_free(ui_sprite_repository);
            return -1;
        }
    }

    if (ui_sprite_repository->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_repository_name(ui_sprite_repository));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_repository_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_repository_t ui_sprite_repository;

    ui_sprite_repository = ui_sprite_repository_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_repository) {
        ui_sprite_repository_free(ui_sprite_repository);
    }
}
