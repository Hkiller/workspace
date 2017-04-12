#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui_sprite_touch_mgr_i.h"
#include "ui_sprite_touch_env_i.h"
#include "ui_sprite_touch_touchable_i.h"
#include "ui_sprite_touch_move_i.h"
#include "ui_sprite_touch_click_i.h"
#include "ui_sprite_touch_scale_i.h"

extern char g_metalib_ui_sprite_touch[];
static void ui_sprite_touch_mgr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_touch_mgr = {
    "ui_sprite_touch_mgr",
    ui_sprite_touch_mgr_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_touch_mgr_t mgr);
    void (*fini)(ui_sprite_touch_mgr_t mgr);
} s_auto_reg_products[] = {
    { "TouchEnv", ui_sprite_touch_env_regist, ui_sprite_touch_env_unregist }
    , { "Touchable", ui_sprite_touch_touchable_regist, ui_sprite_touch_touchable_unregist }
    , { "touch-move", ui_sprite_touch_move_regist, ui_sprite_touch_move_unregist }
    , { "touch-click", ui_sprite_touch_click_regist, ui_sprite_touch_click_unregist }
    , { "touch-scale", ui_sprite_touch_scale_regist, ui_sprite_touch_scale_unregist }
};

#define UI_SPRITE_TOUCH_MGR_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_ui_sprite_touch, __name); \
    assert(module-> __arg)


ui_sprite_touch_mgr_t
ui_sprite_touch_mgr_create(
    gd_app_context_t app, ui_sprite_repository_t repo,
    ui_sprite_fsm_module_t fsm_module, ui_sprite_cfg_loader_t loader,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_touch_mgr * module;
    nm_node_t module_node;
    int component_pos;

    assert(app);

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_touch_mgr));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_touch_mgr_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_repo = repo;
    module->m_fsm = fsm_module;
    module->m_loader = loader;
    module->m_debug = 0;
    module->m_dft_threshold = 3;

    UI_SPRITE_TOUCH_MGR_LOAD_META(m_meta_pos_binding, "ui_sprite_touch_pos_binding");
    UI_SPRITE_TOUCH_MGR_LOAD_META(m_meta_move_state, "ui_sprite_touch_move_state");
    UI_SPRITE_TOUCH_MGR_LOAD_META(m_meta_scale_state, "ui_sprite_touch_scale_state");

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);

            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            nm_node_from_data(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_touch_mgr);
    return module;
}

static void ui_sprite_touch_mgr_clear(nm_node_t node) {
    ui_sprite_touch_mgr_t module;
    int component_pos;

    module = (ui_sprite_touch_mgr_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    ui_sprite_touch_touchable_unregist(module);
}

gd_app_context_t ui_sprite_touch_mgr_app(ui_sprite_touch_mgr_t module) {
    return module->m_app;
}

void ui_sprite_touch_mgr_free(ui_sprite_touch_mgr_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_touch_mgr) return;
    nm_node_free(module_node);
}

ui_sprite_touch_mgr_t
ui_sprite_touch_mgr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_touch_mgr) return NULL;
    return (ui_sprite_touch_mgr_t)nm_node_data(node);
}

ui_sprite_touch_mgr_t
ui_sprite_touch_mgr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_touch_mgr";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_touch_mgr) return NULL;
    return (ui_sprite_touch_mgr_t)nm_node_data(node);
}

const char * ui_sprite_touch_mgr_name(ui_sprite_touch_mgr_t module) {
    return nm_node_name(nm_node_from_data(module));
}

tl_time_t ui_sprite_touch_mgr_cur_time(ui_sprite_touch_mgr_t mgr) {
    return tl_manage_time(gd_app_tl_mgr(mgr->m_app));
}

EXPORT_DIRECTIVE
int ui_sprite_touch_mgr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_touch_mgr_t ui_sprite_touch_mgr;
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

    ui_sprite_touch_mgr =
        ui_sprite_touch_mgr_create(
            app, repo, fsm_module, ui_sprite_cfg_loader_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_touch_mgr == NULL) return -1;

    ui_sprite_touch_mgr->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_touch_mgr->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_touch_mgr_name(ui_sprite_touch_mgr));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_touch_mgr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_touch_mgr_t ui_sprite_touch_mgr;

    ui_sprite_touch_mgr = ui_sprite_touch_mgr_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_touch_mgr) {
        ui_sprite_touch_mgr_free(ui_sprite_touch_mgr);
    }
}

