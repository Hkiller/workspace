#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui_sprite_fsm_module_i.h"
#include "ui_sprite_fsm_component_i.h"
#include "ui_sprite_fsm_action_meta_i.h"
#include "ui_sprite_fsm_action_fsm_i.h"
#include "ui_sprite_fsm_ins_action_i.h"

extern char g_metalib_ui_sprite_fsm[];
static void ui_sprite_fsm_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_fsm_module = {
    "ui_sprite_fsm_module",
    ui_sprite_fsm_module_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_fsm_module_t module);
    void (*fini)(ui_sprite_fsm_module_t module);
} s_auto_reg_products[] = {
    { "fsm-component", ui_sprite_fsm_component_regist, ui_sprite_fsm_component_unregist }
    , { "run-fsm", ui_sprite_fsm_action_fsm_regist, ui_sprite_fsm_action_fsm_unregist }
};

#define UI_SPRITE_FSM_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_ui_sprite_fsm, __name); \
    assert(module-> __arg)

ui_sprite_fsm_module_t
ui_sprite_fsm_module_create(
    gd_app_context_t app, ui_sprite_repository_t repo,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_fsm_module * module;
    nm_node_t module_node;
    int8_t component_pos = 0;

    assert(app);

    if (name == NULL) name = "ui_sprite_fsm_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_fsm_module));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_fsm_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_repo = repo;
    module->m_debug = 0;

    UI_SPRITE_FSM_MODULE_LOAD_META(m_meta_action_enter_event, "ui_sprite_fsm_state_enter_event");

    TAILQ_INIT(&module->m_op_actions);

    if (cpe_hash_table_init(
            &module->m_fsm_action_metas,
            alloc,
            (cpe_hash_fun_t) ui_sprite_fsm_action_meta_hash,
            (cpe_hash_eq_t) ui_sprite_fsm_action_meta_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_fsm_action_meta, m_hh_for_module),
            -1) != 0)
    {
        nm_node_free(module_node);
        return NULL;
    }

    if (ui_sprite_repository_register_events_by_prefix(
            repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_fsm, "ui_sprite_evt") != 0)
    {
        CPE_ERROR(em, "%s: regist events fail!", name);
        cpe_hash_table_fini(&module->m_fsm_action_metas);
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
                repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_fsm, "ui_sprite_evt");
            cpe_hash_table_fini(&module->m_fsm_action_metas);
            nm_node_free(module_node);

            return NULL;
        }
    }

    mem_buffer_init(&module->m_dump_buffer, module->m_alloc);

    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_fsm_module);

    return module;
}

static void ui_sprite_fsm_module_clear(nm_node_t node) {
    ui_sprite_fsm_module_t module;
    int8_t component_pos = 0;

    module = (ui_sprite_fsm_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    ui_sprite_repository_unregister_events_by_prefix(
        module->m_repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_fsm, "ui_sprite_evt");

    assert(TAILQ_EMPTY(&module->m_op_actions));

    /*meta*/
    ui_sprite_fsm_action_meta_free_all(module);
    cpe_hash_table_fini(&module->m_fsm_action_metas);

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t ui_sprite_fsm_module_app(ui_sprite_fsm_module_t module) {
    return module->m_app;
}

void ui_sprite_fsm_module_free(ui_sprite_fsm_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_fsm_module) return;
    nm_node_free(module_node);
}

ui_sprite_fsm_module_t
ui_sprite_fsm_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_fsm_module) return NULL;
    return (ui_sprite_fsm_module_t)nm_node_data(node);
}

ui_sprite_fsm_module_t
ui_sprite_fsm_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_fsm_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_fsm_module) return NULL;
    return (ui_sprite_fsm_module_t)nm_node_data(node);
}

const char * ui_sprite_fsm_module_name(ui_sprite_fsm_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int ui_sprite_fsm_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_fsm_module_t ui_sprite_fsm_module;
    ui_sprite_repository_t repo;

    repo = ui_sprite_repository_find_nc(app, cfg_get_string(cfg, "ui-sprite-repository", "ui_sprite_repository"));
    if (repo == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: repository %s not exist",
            gd_app_module_name(module),
            cfg_get_string(cfg, "ui-sprite-repository", "ui_sprite_repository"));
        return -1;
    }

    ui_sprite_fsm_module =
        ui_sprite_fsm_module_create(
            app, repo,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_fsm_module == NULL) return -1;

    ui_sprite_fsm_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_fsm_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_fsm_module_name(ui_sprite_fsm_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_fsm_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_fsm_module_t ui_sprite_fsm_module;

    ui_sprite_fsm_module = ui_sprite_fsm_module_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_fsm_module) {
        ui_sprite_fsm_module_free(ui_sprite_fsm_module);
    }
}

