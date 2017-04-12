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
#include "ui_sprite_ctrl_module_i.h"
#include "ui_sprite_ctrl_track_mgr_i.h"
#include "ui_sprite_ctrl_circle_i.h"
#include "ui_sprite_ctrl_turntable_i.h"
#include "ui_sprite_ctrl_turntable_member_i.h"
#include "ui_sprite_ctrl_turntable_join_i.h"
#include "ui_sprite_ctrl_turntable_touch_i.h"
#include "ui_sprite_ctrl_turntable_active_i.h"
#include "ui_sprite_ctrl_track_follow_i.h"
#include "ui_sprite_ctrl_track_manip_i.h"

extern char g_metalib_ui_sprite_ctrl[];
static void ui_sprite_ctrl_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_ctrl_module = {
    "ui_sprite_ctrl_module",
    ui_sprite_ctrl_module_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_ctrl_module_t module);
    void (*fini)(ui_sprite_ctrl_module_t module);
} s_auto_reg_products[] = {
    { "ctrl-track-mgr", ui_sprite_ctrl_track_mgr_regist, ui_sprite_ctrl_track_mgr_unregist }
    , { "ctrl-circle", ui_sprite_ctrl_circle_regist, ui_sprite_ctrl_circle_unregist }
    , { "ctrl-turntable", ui_sprite_ctrl_turntable_regist, ui_sprite_ctrl_turntable_unregist }
    , { "ctrl-turntable-member", ui_sprite_ctrl_turntable_member_regist, ui_sprite_ctrl_turntable_member_unregist }
    , { "ctrl-turntable-join", ui_sprite_ctrl_turntable_join_regist, ui_sprite_ctrl_turntable_join_unregist }
    , { "ctrl-turntable-touch", ui_sprite_ctrl_turntable_touch_regist, ui_sprite_ctrl_turntable_touch_unregist }
    , { "ctrl-turntable-active", ui_sprite_ctrl_turntable_active_regist, ui_sprite_ctrl_turntable_active_unregist }
    , { "ctrl-track-manip", ui_sprite_ctrl_track_manip_regist, ui_sprite_ctrl_track_manip_unregist }
    , { "ctrl-track-follow", ui_sprite_ctrl_track_follow_regist, ui_sprite_ctrl_track_follow_unregist }
};

#define UI_SPRITE_CTRL_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_ui_sprite_ctrl, __name); \
    assert(module-> __arg)

ui_sprite_ctrl_module_t
ui_sprite_ctrl_module_create(
    gd_app_context_t app, ui_sprite_repository_t repo,
    ui_sprite_fsm_module_t fsm_module, ui_sprite_cfg_loader_t loader,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_ctrl_module * module;
    nm_node_t module_node;
    int8_t component_pos = 0;

    assert(app);

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_ctrl_module));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_ctrl_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_repo = repo;
    module->m_fsm_module = fsm_module;
    module->m_loader = loader;
    module->m_debug = 0;

    UI_SPRITE_CTRL_MODULE_LOAD_META(m_meta_circle_state, "ui_sprite_ctrl_circle_state");
    UI_SPRITE_CTRL_MODULE_LOAD_META(m_meta_turntable_data, "ui_sprite_ctrl_turntable_data");

    if (ui_sprite_repository_register_events_by_prefix(
            repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_ctrl, "ui_sprite_evt") != 0)
    {
        CPE_ERROR(em, "%s: register events fail!", name);
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
                repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_ctrl, "ui_sprite_evt");

            nm_node_from_data(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_ctrl_module);

    return module;
}

static void ui_sprite_ctrl_module_clear(nm_node_t node) {
    ui_sprite_ctrl_module_t module;
    int component_pos;

    module = (ui_sprite_ctrl_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    ui_sprite_repository_unregister_events_by_prefix(
        module->m_repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_ctrl, "ui_sprite_evt");
}

gd_app_context_t ui_sprite_ctrl_module_app(ui_sprite_ctrl_module_t module) {
    return module->m_app;
}

void ui_sprite_ctrl_module_free(ui_sprite_ctrl_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_ctrl_module) return;
    nm_node_free(module_node);
}

ui_sprite_ctrl_module_t
ui_sprite_ctrl_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_ctrl_module) return NULL;
    return (ui_sprite_ctrl_module_t)nm_node_data(node);
}

ui_sprite_ctrl_module_t
ui_sprite_ctrl_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_ctrl_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_ctrl_module) return NULL;
    return (ui_sprite_ctrl_module_t)nm_node_data(node);
}

const char * ui_sprite_ctrl_module_name(ui_sprite_ctrl_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int ui_sprite_ctrl_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_ctrl_module_t ui_sprite_ctrl_module;
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

    ui_sprite_ctrl_module =
        ui_sprite_ctrl_module_create(
            app, repo, fsm_module, ui_sprite_cfg_loader_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_ctrl_module == NULL) return -1;

    ui_sprite_ctrl_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_ctrl_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_ctrl_module_name(ui_sprite_ctrl_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_ctrl_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_ctrl_module_t ui_sprite_ctrl_module;

    ui_sprite_ctrl_module = ui_sprite_ctrl_module_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_ctrl_module) {
        ui_sprite_ctrl_module_free(ui_sprite_ctrl_module);
    }
}

