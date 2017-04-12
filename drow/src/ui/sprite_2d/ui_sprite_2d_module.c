#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_2d_module_i.h"
#include "ui_sprite_2d_transform_i.h"
#include "ui_sprite_2d_part_i.h"
#include "ui_sprite_2d_part_attr_i.h"
#include "ui_sprite_2d_part_binding_i.h"
#include "ui_sprite_2d_move_i.h"
#include "ui_sprite_2d_rotate_i.h"
#include "ui_sprite_2d_scale_i.h"
#include "ui_sprite_2d_flip_i.h"
#include "ui_sprite_2d_track_flip_i.h"
#include "ui_sprite_2d_track_angle_i.h"
#include "ui_sprite_2d_wait_switchback_i.h"
#include "ui_sprite_2d_wait_stop_i.h"
#include "ui_sprite_2d_wait_move_distance_i.h"
#include "ui_sprite_2d_search_i.h"
#include "ui_sprite_2d_action_part_follow_i.h"

extern char g_metalib_ui_sprite_2d[];
static void ui_sprite_2d_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_2d_module = {
    "ui_sprite_2d_module",
    ui_sprite_2d_module_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_2d_module_t module);
    void (*fini)(ui_sprite_2d_module_t module);
} s_auto_reg_products[] = {
    { "transform", ui_sprite_2d_transform_regist, ui_sprite_2d_transform_unregist }
    , { "2d-move", ui_sprite_2d_move_regist, ui_sprite_2d_move_unregist }
    , { "2d-rotate", ui_sprite_2d_rotate_regist, ui_sprite_2d_rotate_unregist }
	, { "2d-scale", ui_sprite_2d_scale_regist, ui_sprite_2d_scale_unregist }
	, { "2d-flip", ui_sprite_2d_flip_regist, ui_sprite_2d_flip_unregist }
	, { "2d-track-flip", ui_sprite_2d_track_flip_regist, ui_sprite_2d_track_flip_unregist }
	, { "2d-track-angle", ui_sprite_2d_track_angle_regist, ui_sprite_2d_track_angle_unregist }
	, { "2d-wait-switchback", ui_sprite_2d_wait_switchback_regist, ui_sprite_2d_wait_switchback_unregist }
	, { "2d-wait-stop", ui_sprite_2d_wait_stop_regist, ui_sprite_2d_wait_stop_unregist }
	, { "2d-wait-move_distance", ui_sprite_2d_wait_move_distance_regist, ui_sprite_2d_wait_move_distance_unregist }
    , { "2d-search", ui_sprite_2d_search_regist, ui_sprite_2d_search_unregist }
    , { "2d-angle-flip-x", ui_sprite_2d_add_angle_flip_x, ui_sprite_2d_remove_angle_flip_x }
    , { "part-follow", ui_sprite_2d_action_part_follow_regist, ui_sprite_2d_action_part_follow_unregist }
};

#define UI_SPRITE_2D_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_ui_sprite_2d, __name); \
    assert(module-> __arg)

ui_sprite_2d_module_t
ui_sprite_2d_module_create(
    gd_app_context_t app, ui_sprite_repository_t repo,
    ui_sprite_fsm_module_t fsm_module, ui_sprite_cfg_loader_t loader,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_2d_module * module;
    nm_node_t module_node;
    int component_pos;

    assert(app);

    if (name == NULL) name = "ui_sprite_2d_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_2d_module));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_2d_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_repo = repo;
    module->m_fsm_module = fsm_module;
    module->m_loader = loader;
    module->m_debug = 0;

    UI_SPRITE_2D_MODULE_LOAD_META(m_meta_transform_data, "ui_sprite_2d_transform");
    UI_SPRITE_2D_MODULE_LOAD_META(m_meta_search_result, "ui_sprite_2d_search_result");

    if (ui_sprite_repository_register_events_by_prefix(
            repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_2d, "ui_sprite_evt") != 0)
    {
        CPE_ERROR(em, "%s: regist 2d transform fail!", name);
        nm_node_from_data(module_node);
        return NULL;
    }

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            ui_sprite_repository_unregister_events_by_prefix(
                module->m_repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_2d, "ui_sprite_evt");

            nm_node_from_data(module_node);
            return NULL;
        }
    }

    TAILQ_INIT(&module->m_free_parts);
    TAILQ_INIT(&module->m_free_part_attrs);
    TAILQ_INIT(&module->m_free_part_bindings);
    
    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_2d_module);

    return module;
}

static void ui_sprite_2d_module_clear(nm_node_t node) {
    ui_sprite_2d_module_t module;
    int component_pos;

    module = (ui_sprite_2d_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    ui_sprite_repository_unregister_events_by_prefix(
        module->m_repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_2d, "ui_sprite_evt");

    while(!TAILQ_EMPTY(&module->m_free_parts)) {
        ui_sprite_2d_part_real_free(module, TAILQ_FIRST(&module->m_free_parts));
    }

    while(!TAILQ_EMPTY(&module->m_free_part_attrs)) {
        ui_sprite_2d_part_attr_real_free(module, TAILQ_FIRST(&module->m_free_part_attrs));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_part_bindings)) {
        ui_sprite_2d_part_binding_real_free(module, TAILQ_FIRST(&module->m_free_part_bindings));
    }
}

gd_app_context_t ui_sprite_2d_module_app(ui_sprite_2d_module_t module) {
    return module->m_app;
}

void ui_sprite_2d_module_free(ui_sprite_2d_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_2d_module) return;
    nm_node_free(module_node);
}

ui_sprite_2d_module_t
ui_sprite_2d_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_2d_module) return NULL;
    return (ui_sprite_2d_module_t)nm_node_data(node);
}

ui_sprite_2d_module_t
ui_sprite_2d_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_2d_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_2d_module) return NULL;
    return (ui_sprite_2d_module_t)nm_node_data(node);
}

const char * ui_sprite_2d_module_name(ui_sprite_2d_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int ui_sprite_2d_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_2d_module_t ui_sprite_2d_module;
    ui_sprite_repository_t repo;
    ui_sprite_fsm_module_t fsm_module;
    
    repo = ui_sprite_repository_find_nc(app, cfg_get_string(cfg, "ui-sprite-repository", "ui_sprite_repository"));
    if (repo == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: repository %s not exist",
            gd_app_module_name(module),
            cfg_get_string(cfg, "ui-sprite-repository", "ui_sprite_repository"));
        return -1;
    }

    fsm_module = ui_sprite_fsm_module_find_nc(app, cfg_get_string(cfg, "ui-sprite-fsm-repository", NULL));
    if (fsm_module == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: fsm-repository %s not exist",
            gd_app_module_name(module),
            cfg_get_string(cfg, "ui-sprite-fsm-repository", "default"));
        return -1;
    }

    ui_sprite_2d_module =
        ui_sprite_2d_module_create(
            app, repo, fsm_module, ui_sprite_cfg_loader_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_2d_module == NULL) return -1;

    ui_sprite_2d_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_2d_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_2d_module_name(ui_sprite_2d_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_2d_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_2d_module_t ui_sprite_2d_module;

    ui_sprite_2d_module = ui_sprite_2d_module_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_2d_module) {
        ui_sprite_2d_module_free(ui_sprite_2d_module);
    }
}

