#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_render/ui_sprite_render_module.h"
#include "ui/sprite_tri/ui_sprite_tri_module.h"
#include "ui_sprite_chipmunk_module_i.h"
#include "ui_sprite_chipmunk_render_creator_i.h"
#include "ui_sprite_chipmunk_env_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_group_i.h"
#include "ui_sprite_chipmunk_on_collision_i.h"
#include "ui_sprite_chipmunk_on_click_i.h"
#include "ui_sprite_chipmunk_send_event_to_collision_i.h"
#include "ui_sprite_chipmunk_wait_collision_i.h"
#include "ui_sprite_chipmunk_with_collision_i.h"
#include "ui_sprite_chipmunk_with_group_i.h"
#include "ui_sprite_chipmunk_with_boundary_i.h"
#include "ui_sprite_chipmunk_with_time_scale_i.h"
#include "ui_sprite_chipmunk_with_gravity_i.h"
#include "ui_sprite_chipmunk_with_runing_mode_i.h"
#include "ui_sprite_chipmunk_with_damping_i.h"
#include "ui_sprite_chipmunk_with_addition_accel_i.h"
#include "ui_sprite_chipmunk_with_constraint_i.h"
#include "ui_sprite_chipmunk_with_attractor_i.h"
#include "ui_sprite_chipmunk_move_to_entity_i.h"
#include "ui_sprite_chipmunk_touch_move_i.h"
#include "ui_sprite_chipmunk_track_angle_i.h"
#include "ui_sprite_chipmunk_manipulator_i.h"
#include "ui_sprite_chipmunk_apply_velocity_i.h"
#include "ui_sprite_chipmunk_tri_scope_i.h"
#include "ui_sprite_chipmunk_tri_have_entity_i.h"
#include "ui_sprite_chipmunk_tri_scope_render_i.h"
#include "ui_sprite_chipmunk_tri_scope_render_creator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char g_metalib_ui_sprite_chipmunk[];
static void ui_sprite_chipmunk_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_chipmunk_module = {
    "ui_sprite_chipmunk_module",
    ui_sprite_chipmunk_module_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_chipmunk_module_t module);
    void (*fini)(ui_sprite_chipmunk_module_t module);
} s_auto_reg_products[] = {
    { "chipmunk-env", ui_sprite_chipmunk_env_regist, ui_sprite_chipmunk_env_unregist }
    , { "chipmunk-render-creator", ui_sprite_chipmunk_render_creator_regist, ui_sprite_chipmunk_render_creator_unregist }
    , { "chipmunk-obj", ui_sprite_chipmunk_obj_regist, ui_sprite_chipmunk_obj_unregist }
    , { "chipmunk-on-collision", ui_sprite_chipmunk_on_collision_regist, ui_sprite_chipmunk_on_collision_unregist }
    , { "chipmunk-on-click", ui_sprite_chipmunk_on_click_regist, ui_sprite_chipmunk_on_click_unregist }
    , { "chipmunk-with-time-scale", ui_sprite_chipmunk_with_time_scale_regist, ui_sprite_chipmunk_with_time_scale_unregist }
    , { "chipmunk-send-event-to-collision", ui_sprite_chipmunk_send_event_to_collision_regist, ui_sprite_chipmunk_send_event_to_collision_unregist }
    , { "chipmunk-wait-collision", ui_sprite_chipmunk_wait_collision_regist, ui_sprite_chipmunk_wait_collision_unregist }
    , { "chipmunk-with-collision", ui_sprite_chipmunk_with_collision_regist, ui_sprite_chipmunk_with_collision_unregist }
    , { "chipmunk-with-group", ui_sprite_chipmunk_with_group_regist, ui_sprite_chipmunk_with_group_unregist }
    , { "chipmunk-with-boundary", ui_sprite_chipmunk_with_boundary_regist, ui_sprite_chipmunk_with_boundary_unregist }
    , { "chipmunk-with-gravity", ui_sprite_chipmunk_with_gravity_regist, ui_sprite_chipmunk_with_gravity_unregist }
    , { "chipmunk-with-runing_mode", ui_sprite_chipmunk_with_runing_mode_regist, ui_sprite_chipmunk_with_runing_mode_unregist }
    , { "chipmunk-with-addition-accel", ui_sprite_chipmunk_with_addition_accel_regist, ui_sprite_chipmunk_with_addition_accel_unregist }
    , { "chipmunk-with-track-angle", ui_sprite_chipmunk_track_angle_regist, ui_sprite_chipmunk_track_angle_unregist }
    , { "chipmunk-with-damping", ui_sprite_chipmunk_with_damping_regist, ui_sprite_chipmunk_with_damping_unregist }
    , { "chipmunk-with-constraint", ui_sprite_chipmunk_with_constraint_regist, ui_sprite_chipmunk_with_constraint_unregist }
    , { "chipmunk-with-attractor", ui_sprite_chipmunk_with_attractor_regist, ui_sprite_chipmunk_with_attractor_unregist }
    , { "chipmunk-move-to-entity", ui_sprite_chipmunk_move_to_entity_regist, ui_sprite_chipmunk_move_to_entity_unregist }
    , { "chipmunk-move-by-touch", ui_sprite_chipmunk_touch_move_regist, ui_sprite_chipmunk_touch_move_unregist }
    , { "chipmunk-manipulator", ui_sprite_chipmunk_manipulator_regist, ui_sprite_chipmunk_manipulator_unregist }
    , { "chipmunk-apply-velocity", ui_sprite_chipmunk_apply_velocity_regist, ui_sprite_chipmunk_apply_velocity_unregist }
    , { "chipmunk-tri-have-entity", ui_sprite_chipmunk_tri_have_entity_regist, ui_sprite_chipmunk_tri_have_entity_unregist }
    , { "chipmunk-tri-scope-render", ui_sprite_chipmunk_tri_scope_render_regist, ui_sprite_chipmunk_tri_scope_render_unregist }
    , { "chipmunk-tri-scope-render-creator", ui_sprite_chipmunk_tri_scope_render_creator_regist, ui_sprite_chipmunk_tri_scope_render_creator_unregist }
};

#define UI_SPRITE_CHIPMUNK_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_ui_sprite_chipmunk, __name); \
    assert(module-> __arg)
    
ui_sprite_chipmunk_module_t
ui_sprite_chipmunk_module_create(
    gd_app_context_t app,
    ui_runtime_module_t runtime,
    ui_sprite_repository_t repo,
    ui_sprite_fsm_module_t fsm_module,
    ui_sprite_cfg_loader_t loader,
    plugin_chipmunk_module_t chipmunk_module,
    ui_sprite_render_module_t sprite_render,
    ui_sprite_tri_module_t tri,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_chipmunk_module * module;
    nm_node_t module_node;
    int8_t component_pos = 0;

    assert(app);

    if (name == NULL) name = "ui_sprite_chipmunk_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_chipmunk_module));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_chipmunk_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_runtime = runtime;
    module->m_repo = repo;
    module->m_fsm_module = fsm_module;
    module->m_loader = loader;
    module->m_chipmunk_module = chipmunk_module;
    module->m_sprite_render = sprite_render;
    module->m_tri = tri;
    module->m_debug = 0;

    UI_SPRITE_CHIPMUNK_MODULE_LOAD_META(m_meta_chipmunk_obj_data, "ui_sprite_chipmunk_obj_data");
    UI_SPRITE_CHIPMUNK_MODULE_LOAD_META(m_meta_chipmunk_collision_data, "ui_sprite_chipmunk_collision_data");
    UI_SPRITE_CHIPMUNK_MODULE_LOAD_META(m_meta_chipmunk_move_state, "ui_sprite_chipmunk_move_state");

    mem_buffer_init(&module->m_dump_buffer, NULL);

    if (ui_sprite_repository_register_events_by_prefix(
            repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_chipmunk, "ui_sprite_evt") != 0)
    {
        CPE_ERROR(em, "%s: regist chipmunk module event fail!", name);
        mem_buffer_clear(&module->m_dump_buffer);
        nm_node_free(module_node);
        return NULL;
    }
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            ui_sprite_repository_unregister_events_by_prefix(
                repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_chipmunk, "ui_sprite_evt");

            mem_buffer_clear(&module->m_dump_buffer);
            nm_node_free(module_node);
            return NULL;
        }
    }

    TAILQ_INIT(&module->m_free_body_group_bindings);
    TAILQ_INIT(&module->m_free_tri_scopes);

    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_chipmunk_module);

    return module;
}

static void ui_sprite_chipmunk_module_clear(nm_node_t node) {
    ui_sprite_chipmunk_module_t module;
    int component_pos;

    module = (ui_sprite_chipmunk_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    ui_sprite_repository_unregister_events_by_prefix(
        module->m_repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_chipmunk, "ui_sprite_evt");

    while(!TAILQ_EMPTY(&module->m_free_body_group_bindings)) {
        ui_sprite_chipmunk_obj_body_group_binding_real_free(TAILQ_FIRST(&module->m_free_body_group_bindings));
    }

    while(!TAILQ_EMPTY(&module->m_free_tri_scopes)) {
        ui_sprite_chipmunk_tri_scope_real_free(TAILQ_FIRST(&module->m_free_tri_scopes));
    }
    
    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t ui_sprite_chipmunk_module_app(ui_sprite_chipmunk_module_t module) {
    return module->m_app;
}

void ui_sprite_chipmunk_module_free(ui_sprite_chipmunk_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_chipmunk_module) return;
    nm_node_free(module_node);
}

ui_sprite_chipmunk_module_t
ui_sprite_chipmunk_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_chipmunk_module) return NULL;
    return (ui_sprite_chipmunk_module_t)nm_node_data(node);
}

ui_sprite_chipmunk_module_t
ui_sprite_chipmunk_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_chipmunk_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_chipmunk_module) return NULL;
    return (ui_sprite_chipmunk_module_t)nm_node_data(node);
}

const char * ui_sprite_chipmunk_module_name(ui_sprite_chipmunk_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

ui_sprite_chipmunk_unit_t ui_sprite_chipmunk_unit_from_str(const char * str) {
    if (strcmp(str, "logic") == 0) {
        return ui_sprite_chipmunk_unit_logic;
    }
    else if (strcmp(str, "pixel") == 0) {
        return ui_sprite_chipmunk_unit_pixel;
    }
    else {
        return ui_sprite_chipmunk_unit_unknown;
    }
}

LPDRMETA ui_sprite_chipmunk_module_collision_data_meta(ui_sprite_chipmunk_module_t module) {
    return module->m_meta_chipmunk_collision_data;
}

tl_time_t ui_sprite_chipmunk_module_cur_time(ui_sprite_chipmunk_module_t module) {
    return tl_manage_time(gd_app_tl_mgr(module->m_app));
}
    
EXPORT_DIRECTIVE
int ui_sprite_chipmunk_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_chipmunk_module_t ui_sprite_chipmunk_module;
    ui_sprite_repository_t repo;
    ui_sprite_fsm_module_t fsm_module;
    ui_sprite_cfg_loader_t loader;
    plugin_chipmunk_module_t chipmunk_module;

    chipmunk_module = plugin_chipmunk_module_find_nc(app, cfg_get_string(cfg, "chipmunk", NULL));
    if (chipmunk_module == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: chipmunk %s not exist",
            gd_app_module_name(module),
            cfg_get_string(cfg, "chipmunk", "default"));
        return -1;
    }

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

    loader = ui_sprite_cfg_loader_find_nc(app, cfg_get_string(cfg, "ui-sprite-cfg-loader", NULL));
    if (loader == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: cfg-loader %s not exist",
            gd_app_module_name(module),
            cfg_get_string(cfg, "ui-sprite-cfg-loader", "default"));
        return -1;
    }

    ui_sprite_chipmunk_module =
        ui_sprite_chipmunk_module_create(
            app,
            ui_runtime_module_find_nc(app, NULL),
            repo, fsm_module, loader,
            chipmunk_module,
            ui_sprite_render_module_find_nc(app, NULL),
            ui_sprite_tri_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_chipmunk_module == NULL) return -1;

    ui_sprite_chipmunk_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_chipmunk_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_chipmunk_module_name(ui_sprite_chipmunk_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_chipmunk_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_chipmunk_module_t ui_sprite_chipmunk_module;

    ui_sprite_chipmunk_module = ui_sprite_chipmunk_module_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_chipmunk_module) {
        ui_sprite_chipmunk_module_free(ui_sprite_chipmunk_module);
    }
}

#ifdef __cplusplus
}
#endif
