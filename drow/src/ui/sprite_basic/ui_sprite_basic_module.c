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
#include "ui_sprite_basic_module_i.h"
#include "ui_sprite_basic_send_event_i.h"
#include "ui_sprite_basic_gen_entities_i.h"
#include "ui_sprite_basic_noop_i.h"
#include "ui_sprite_basic_debug_i.h"
#include "ui_sprite_basic_join_group_i.h"
#include "ui_sprite_basic_set_attrs_i.h"
#include "ui_sprite_basic_guard_attrs_i.h"
#include "ui_sprite_basic_wait_event_i.h"
#include "ui_sprite_basic_wait_condition_i.h"
#include "ui_sprite_basic_wait_entity_destory_i.h"
#include "ui_sprite_basic_wait_entity_in_state_i.h"
#include "ui_sprite_basic_wait_entity_not_in_state_i.h"
#include "ui_sprite_basic_destory_self_i.h"

extern char g_metalib_ui_sprite_basic[];
static void ui_sprite_basic_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_basic_module = {
    "ui_sprite_basic_module",
    ui_sprite_basic_module_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_basic_module_t module);
    void (*fini)(ui_sprite_basic_module_t module);
} s_auto_reg_products[] = {
    { "noop", ui_sprite_basic_noop_regist, ui_sprite_basic_noop_unregist }
    , { "debug", ui_sprite_basic_debug_regist, ui_sprite_basic_debug_unregist }
    , { "send-event", ui_sprite_basic_send_event_regist, ui_sprite_basic_send_event_unregist }
    , { "wait-event", ui_sprite_basic_wait_event_regist, ui_sprite_basic_wait_event_unregist }
    , { "wait-condition", ui_sprite_basic_wait_condition_regist, ui_sprite_basic_wait_condition_unregist }
    , { "gen-entitites", ui_sprite_basic_gen_entities_regist, ui_sprite_basic_gen_entities_unregist }
    , { "join-group", ui_sprite_basic_join_group_regist, ui_sprite_basic_join_group_unregist }
    , { "set-attrs", ui_sprite_basic_set_attrs_regist, ui_sprite_basic_set_attrs_unregist }
    , { "guard-attrs", ui_sprite_basic_guard_attrs_regist, ui_sprite_basic_guard_attrs_unregist }
	, { "check-entity-exist", ui_sprite_basic_wait_entity_destory_regist, ui_sprite_basic_wait_entity_destory_unregist }
	, { "check-entity-in-state", ui_sprite_basic_wait_entity_in_state_regist, ui_sprite_basic_wait_entity_in_state_unregist }
	, { "check-entity-not-in-state", ui_sprite_basic_wait_entity_not_in_state_regist, ui_sprite_basic_wait_entity_not_in_state_unregist }
	, { "destory-self", ui_sprite_basic_destory_self_regist, ui_sprite_basic_destory_self_unregist }
};

#define UI_SPRITE_BASIC_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_ui_sprite_basic, __name); \
    assert(module-> __arg)


ui_sprite_basic_module_t
ui_sprite_basic_module_create(
    gd_app_context_t app, ui_sprite_repository_t repo, ui_sprite_fsm_module_t fsm_module, ui_sprite_cfg_loader_t loader,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_basic_module * module;
    nm_node_t module_node;
    int component_pos;

    assert(app);

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_basic_module));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_basic_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_repo = repo;
    module->m_fsm_module = fsm_module;
    module->m_loader = loader;
    module->m_debug = 0;

    UI_SPRITE_BASIC_MODULE_LOAD_META(m_meta_gen_entities_data, "ui_sprite_basic_gen_entities_data");

    if (ui_sprite_repository_register_events_by_prefix(
            repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_basic, "ui_sprite_evt") != 0)
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
                repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_basic, "ui_sprite_evt");

            nm_node_from_data(module_node);
            return NULL;
        }
    }

    mem_buffer_init(&module->m_tmp_buff, alloc);

    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_basic_module);

    return module;
}

static void ui_sprite_basic_module_clear(nm_node_t node) {
    ui_sprite_basic_module_t module;
    int component_pos;

    module = (ui_sprite_basic_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    ui_sprite_repository_unregister_events_by_prefix(
        module->m_repo, (LPDRMETALIB)(void*)g_metalib_ui_sprite_basic, "ui_sprite_evt");

    mem_buffer_clear(&module->m_tmp_buff);
}

gd_app_context_t ui_sprite_basic_module_app(ui_sprite_basic_module_t module) {
    return module->m_app;
}

void ui_sprite_basic_module_free(ui_sprite_basic_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_basic_module) return;
    nm_node_free(module_node);
}

ui_sprite_basic_module_t
ui_sprite_basic_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_basic_module) return NULL;
    return (ui_sprite_basic_module_t)nm_node_data(node);
}

ui_sprite_basic_module_t
ui_sprite_basic_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_basic_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_basic_module) return NULL;
    return (ui_sprite_basic_module_t)nm_node_data(node);
}

const char * ui_sprite_basic_module_name(ui_sprite_basic_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int ui_sprite_basic_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_basic_module_t ui_sprite_basic_module;
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

    ui_sprite_basic_module =
        ui_sprite_basic_module_create(
            app, repo, fsm_module, ui_sprite_cfg_loader_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_basic_module == NULL) return -1;

    ui_sprite_basic_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_basic_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_basic_module_name(ui_sprite_basic_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_basic_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_basic_module_t ui_sprite_basic_module;

    ui_sprite_basic_module = ui_sprite_basic_module_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_basic_module) {
        ui_sprite_basic_module_free(ui_sprite_basic_module);
    }
}

