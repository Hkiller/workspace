#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_module.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_render/ui_sprite_render_module.h"
#include "ui/sprite_tri/ui_sprite_tri_module.h"
#include "ui_sprite_spine_module_i.h"
#include "ui_sprite_spine_bind_parts_i.h"
#include "ui_sprite_spine_control_entity_i.h"
#include "ui_sprite_spine_controled_obj_i.h"
#include "ui_sprite_spine_follow_parts_i.h"
#include "ui_sprite_spine_ik_restore_i.h"
#include "ui_sprite_spine_play_anim_i.h"
#include "ui_sprite_spine_schedule_state_i.h"
#include "ui_sprite_spine_apply_transition_i.h" 
#include "ui_sprite_spine_guard_transition_i.h"
#include "ui_sprite_spine_set_state_i.h"
#include "ui_sprite_spine_move_entity_i.h"
#include "ui_sprite_spine_tri_apply_transition_i.h"
#include "ui_sprite_spine_tri_set_timescale_i.h"
#include "ui_sprite_spine_tri_on_part_state_i.h"

//extern char g_metalib_ui_sprite_spine[];
static void ui_sprite_spine_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_spine_module = {
    "ui_sprite_spine_module",
    ui_sprite_spine_module_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_spine_module_t module);
    void (*fini)(ui_sprite_spine_module_t module);
} s_auto_reg_products[] = {
    { "spine-play-anim", ui_sprite_spine_play_anim_regist, ui_sprite_spine_play_anim_unregist }
    , { "spine-schedule-state", ui_sprite_spine_schedule_state_regist, ui_sprite_spine_schedule_state_unregist }
    , { "spine-apply-transition", ui_sprite_spine_apply_transition_regist, ui_sprite_spine_apply_transition_unregist }
    , { "spine-guard-transition", ui_sprite_spine_guard_transition_regist, ui_sprite_spine_guard_transition_unregist }
    , { "spine-set-state", ui_sprite_spine_set_state_regist, ui_sprite_spine_set_state_unregist }
    , { "spine-bind-parts", ui_sprite_spine_bind_parts_regist, ui_sprite_spine_bind_parts_unregist }
    , { "spine-control-entity", ui_sprite_spine_control_entity_regist, ui_sprite_spine_control_entity_unregist }
    , { "spine-controled-obj", ui_sprite_spine_controled_obj_regist, ui_sprite_spine_controled_obj_unregist }
    , { "spine-follow-parts", ui_sprite_spine_follow_parts_regist, ui_sprite_spine_follow_parts_unregist }
    , { "spine-ik-restore", ui_sprite_spine_ik_restore_regist, ui_sprite_spine_ik_restore_unregist }
    , { "spine-move-entity", ui_sprite_spine_move_entity_regist, ui_sprite_spine_move_entity_unregist }
    , { "spine-tri-apply-transition", ui_sprite_spine_tri_apply_transition_regist, ui_sprite_spine_tri_apply_transition_unregist }
    , { "spine-tri-set-timescale", ui_sprite_spine_tri_set_timescale_regist, ui_sprite_spine_tri_set_timescale_unregist }
    , { "spine-tri-on-part-state", ui_sprite_spine_tri_on_part_state_regist, ui_sprite_spine_tri_on_part_state_unregist }
};

ui_sprite_spine_module_t
ui_sprite_spine_module_create(
    gd_app_context_t app, ui_runtime_module_t runtime, ui_sprite_repository_t repo, ui_sprite_fsm_module_t fsm_module,
    ui_sprite_cfg_loader_t loader, ui_sprite_render_module_t sprite_render, ui_sprite_tri_module_t tri,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_spine_module * module;
    nm_node_t module_node;
    int8_t component_pos = 0;

    assert(app);

    if (name == NULL) name = "ui_sprite_spine_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_spine_module));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_spine_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_runtime = runtime;
    module->m_repo = repo;
    module->m_fsm_module = fsm_module;
    module->m_loader = loader;
    module->m_sprite_render = sprite_render;
    module->m_tri = tri;
    module->m_debug = 0;

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

    TAILQ_INIT(&module->m_free_bind_parts_binding);
    TAILQ_INIT(&module->m_free_control_entity_slots);
    TAILQ_INIT(&module->m_free_follow_parts_binding);
    mem_buffer_init(&module->m_dump_buffer, module->m_alloc);
    
    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_spine_module);

    return module;
}

static void ui_sprite_spine_module_clear(nm_node_t node) {
    ui_sprite_spine_module_t module;
    int component_pos;

    module = (ui_sprite_spine_module_t)nm_node_data(node);
    
    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    while(!TAILQ_EMPTY(&module->m_free_bind_parts_binding)) {
        ui_sprite_spine_bind_parts_binding_real_free(TAILQ_FIRST(&module->m_free_bind_parts_binding));
    }

    while(!TAILQ_EMPTY(&module->m_free_control_entity_slots)) {
        ui_sprite_spine_control_entity_slot_real_free(TAILQ_FIRST(&module->m_free_control_entity_slots));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_follow_parts_binding)) {
        ui_sprite_spine_follow_parts_binding_real_free(TAILQ_FIRST(&module->m_free_follow_parts_binding));
    }

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t ui_sprite_spine_module_app(ui_sprite_spine_module_t module) {
    return module->m_app;
}

void ui_sprite_spine_module_free(ui_sprite_spine_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_spine_module) return;
    nm_node_free(module_node);
}

ui_sprite_spine_module_t
ui_sprite_spine_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_spine_module) return NULL;
    return (ui_sprite_spine_module_t)nm_node_data(node);
}

ui_sprite_spine_module_t
ui_sprite_spine_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_spine_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_spine_module) return NULL;
    return (ui_sprite_spine_module_t)nm_node_data(node);
}

const char * ui_sprite_spine_module_name(ui_sprite_spine_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

const char * ui_sprite_spine_module_analize_name(ui_sprite_spine_module_t module, const char * name, char * * args) {
    char * sep;
    char * buf;
    char * end;
    
    if (args) *args = NULL;
    
    sep = strchr(name, '[');
    if (sep == NULL) {
        return name;
    }

    mem_buffer_clear_data(&module->m_dump_buffer);
    buf = mem_buffer_strdup(&module->m_dump_buffer, name);
    assert(buf);
    if (buf == NULL) return NULL;
    
    sep = buf + (sep - name);
    *sep = 0;

    sep += 1;
    end = strchr(sep, ']');
    if (end == NULL) {
        if (args) *args = NULL;
        return buf;
    }

    *end = 0;
    if (args) *args = sep;
    
    return buf;
}

EXPORT_DIRECTIVE
int ui_sprite_spine_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_spine_module_t ui_sprite_spine_module;
    ui_runtime_module_t runtime;
    ui_sprite_repository_t repo;
    ui_sprite_fsm_module_t fsm_module;
    ui_sprite_cfg_loader_t loader;

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
    
    ui_sprite_spine_module =
        ui_sprite_spine_module_create(
            app, runtime, repo, fsm_module, loader,
            ui_sprite_render_module_find_nc(app, NULL),
            ui_sprite_tri_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_spine_module == NULL) return -1;

    ui_sprite_spine_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_spine_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_spine_module_name(ui_sprite_spine_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_spine_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_spine_module_t ui_sprite_spine_module;

    ui_sprite_spine_module = ui_sprite_spine_module_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_spine_module) {
        ui_sprite_spine_module_free(ui_sprite_spine_module);
    }
}

