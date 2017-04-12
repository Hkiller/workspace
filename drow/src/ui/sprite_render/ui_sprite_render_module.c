#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_module.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui_sprite_render_module_i.h"
#include "ui_sprite_render_env_i.h"
#include "ui_sprite_render_sch_i.h"
#include "ui_sprite_render_obj_creator_i.h"
#include "ui_sprite_render_action_adj_priority_i.h"
#include "ui_sprite_render_lock_on_screen_i.h"
#include "ui_sprite_render_show_animation_i.h"
#include "ui_sprite_render_with_obj_i.h"
#include "ui_sprite_render_change_second_color_i.h"
#include "ui_sprite_render_suspend_i.h"
#include "ui_sprite_render_resume_i.h"
#include "ui_sprite_render_action_obj_alpha_out_i.h"
#include "ui_sprite_render_action_obj_alpha_in_i.h"
#include "ui_sprite_render_action_obj_bind_value_i.h"
#include "ui_sprite_render_entity_render_obj_creator_i.h"
#include "ui_sprite_render_obj_world_i.h"
#include "ui_sprite_render_obj_entity_i.h"

static void ui_sprite_render_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_render_sprite_render_module = {
    "ui_sprite_render_module",
    ui_sprite_render_module_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_render_module_t module);
    void (*fini)(ui_sprite_render_module_t module);
} s_auto_reg_products[] = {
    { "suspend-monitor", ui_sprite_render_suspend_monitor_regist, ui_sprite_render_suspend_monitor_unregist }
    , { "render-env", ui_sprite_render_env_regist, ui_sprite_render_env_unregist }
    , { "render-sch", ui_sprite_render_sch_regist, ui_sprite_render_sch_unregist }
    , { "entity-render-obj-creator", ui_sprite_render_entity_render_obj_creator_regist, ui_sprite_render_entity_render_obj_creator_unregist }
    , { "render-obj-world", ui_sprite_render_obj_world_regist, ui_sprite_render_obj_world_unregist }
    , { "render-obj-entity", ui_sprite_render_obj_entity_regist, ui_sprite_render_obj_entity_unregist }
    , { "render-show-animation", ui_sprite_render_show_animation_regist, ui_sprite_render_show_animation_unregist }
    , { "render-adj-priority", ui_sprite_render_action_adj_priority_regist, ui_sprite_render_action_adj_priority_unregist }
    , { "render-lock-on-screen", ui_sprite_render_lock_on_screen_regist, ui_sprite_render_lock_on_screen_unregist }
    , { "render-with-obj", ui_sprite_render_with_obj_regist, ui_sprite_render_with_obj_unregist }
    , { "render-change-second-color", ui_sprite_render_change_second_color_regist, ui_sprite_render_change_second_color_unregist }
    , { "render-suspend", ui_sprite_render_suspend_regist, ui_sprite_render_suspend_unregist }
    , { "render-resume", ui_sprite_render_resume_regist, ui_sprite_render_resume_unregist }
    , { "render-obj-bind-value", ui_sprite_render_action_obj_bind_value_regist, ui_sprite_render_action_obj_bind_value_unregist }
    , { "render-obj-alpha-out", ui_sprite_render_action_obj_alpha_out_regist, ui_sprite_render_action_obj_alpha_out_unregist }
    , { "render-obj-alpha-in", ui_sprite_render_action_obj_alpha_in_regist, ui_sprite_render_action_obj_alpha_in_unregist }
};

ui_sprite_render_module_t
ui_sprite_render_module_create(
    gd_app_context_t app, ui_sprite_repository_t repo, ui_sprite_fsm_module_t fsm_module, ui_sprite_cfg_loader_t loader,
    ui_runtime_module_t runtime, mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_render_module * module;
    nm_node_t module_node;
    int8_t component_pos = 0;

    assert(app);

    if (name == NULL) name = "ui_sprite_render_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_render_module));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_render_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_repo = repo;
    module->m_fsm_module = fsm_module;
    module->m_loader = loader;
    module->m_runtime = runtime;
    module->m_suspend_monitor = NULL;
    module->m_app_pause = 0;
    
    if (cpe_hash_table_init(
            &module->m_obj_creators,
            module->m_alloc,
            (cpe_hash_fun_t) ui_sprite_render_obj_creator_hash,
            (cpe_hash_eq_t) ui_sprite_render_obj_creator_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_render_obj_creator, m_hh),
            -1) != 0)
    {
        CPE_ERROR(module->m_em, "%s: init ext obj creator hash table fail!", name);
        nm_node_from_data(module_node);
        return NULL;
    }

    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }
            cpe_hash_table_fini(&module->m_obj_creators);
            nm_node_from_data(module_node);
            return NULL;
        }
    }

    mem_buffer_init(&module->m_tmp_buffer, module->m_alloc);
    
    nm_node_set_type(module_node, &s_nm_node_type_render_sprite_render_module);

    return module;
}

static void ui_sprite_render_module_clear(nm_node_t node) {
    ui_sprite_render_module_t module;
    int component_pos;

    module = (ui_sprite_render_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    mem_buffer_clear(&module->m_tmp_buffer);

    ui_sprite_render_obj_creator_free_all(module);
    cpe_hash_table_fini(&module->m_obj_creators);
}

gd_app_context_t ui_sprite_render_module_app(ui_sprite_render_module_t module) {
    return module->m_app;
}

void ui_sprite_render_module_free(ui_sprite_render_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_render_sprite_render_module) return;
    nm_node_free(module_node);
}

ui_sprite_render_module_t
ui_sprite_render_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_render_sprite_render_module) return NULL;
    return (ui_sprite_render_module_t)nm_node_data(node);
}

ui_sprite_render_module_t
ui_sprite_render_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_render_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_render_sprite_render_module) return NULL;
    return (ui_sprite_render_module_t)nm_node_data(node);
}

const char * ui_sprite_render_module_name(ui_sprite_render_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

ui_runtime_module_t ui_sprite_render_module_runtime(ui_sprite_render_module_t module) {
    return module->m_runtime;
}

EXPORT_DIRECTIVE
int ui_sprite_render_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_render_module_t ui_sprite_render_module;
    ui_sprite_repository_t repo;
    ui_sprite_fsm_module_t fsm_module;
    ui_sprite_cfg_loader_t loader;
    ui_runtime_module_t runtime;

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

    runtime = ui_runtime_module_find_nc(app, NULL);
    if (runtime == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: runtime not exist", gd_app_module_name(module));
        return -1;
    }

    ui_sprite_render_module =
        ui_sprite_render_module_create(
            app, repo, fsm_module, loader,
            runtime,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_render_module == NULL) return -1;

    ui_sprite_render_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_render_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_render_module_name(ui_sprite_render_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_render_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_render_module_t ui_sprite_render_module;

    ui_sprite_render_module = ui_sprite_render_module_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_render_module) {
        ui_sprite_render_module_free(ui_sprite_render_module);
    }
}

