#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/editor/plugin_editor_module.h"
#include "plugin_ui_module_i.h"
#include "plugin_ui_page_meta_i.h"
#include "plugin_ui_animation_meta_i.h"
#include "plugin_ui_anim_control_alpha_in_i.h"
#include "plugin_ui_anim_control_alpha_out_i.h"
#include "plugin_ui_anim_control_move_i.h"
#include "plugin_ui_anim_control_move_in_i.h"
#include "plugin_ui_anim_control_move_out_i.h"
#include "plugin_ui_anim_control_scale_i.h"
#include "plugin_ui_anim_control_scale_in_i.h"
#include "plugin_ui_anim_control_scale_out_i.h"
#include "plugin_ui_anim_control_anim_i.h"
#include "plugin_ui_anim_control_scroll_i.h"
#include "plugin_ui_anim_control_frame_scale_i.h"
#include "plugin_ui_anim_control_frame_move_i.h"
#include "plugin_ui_anim_label_time_duration_i.h"
#include "plugin_ui_anim_progress_bind_control_i.h"
#include "plugin_ui_anim_frame_move_i.h"
#include "plugin_ui_anim_frame_bind_i.h"
#include "plugin_ui_anim_frame_alpha_i.h"
#include "plugin_ui_control_meta_i.h"
#include "plugin_ui_move_algorithm_meta_i.h"
#include "plugin_ui_move_algorithm_linear_i.h"
#include "plugin_ui_move_algorithm_oval_quarter_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_template_render_i.h"

extern char g_metalib_plugin_ui[];
static void plugin_ui_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_ui_module = {
    "plugin_ui_module",
    plugin_ui_module_clear
};

static struct {
    const char * name; 
    int (*init)(plugin_ui_module_t module);
    void (*fini)(plugin_ui_module_t module);
} s_auto_reg_products[] = {
    { "control-meta-buf", plugin_ui_control_meta_buff_init, plugin_ui_control_meta_buff_fini }
    , { "ui-control-metfa-page", plugin_ui_control_meta_page_regist, plugin_ui_control_meta_page_unregist }
    , { "ui-anim-control-alpha-in", plugin_ui_anim_control_alpha_in_regist, plugin_ui_anim_control_alpha_in_unregist }
    , { "ui-anim-control-alpha-out", plugin_ui_anim_control_alpha_out_regist, plugin_ui_anim_control_alpha_out_unregist }
    , { "ui-anim-control-move-in", plugin_ui_anim_control_move_in_regist, plugin_ui_anim_control_move_in_unregist }
    , { "ui-anim-control-move-out", plugin_ui_anim_control_move_out_regist, plugin_ui_anim_control_move_out_unregist }
    , { "ui-anim-control-scale", plugin_ui_anim_control_scale_regist, plugin_ui_anim_control_scale_unregist }
    , { "ui-anim-control-scale-in", plugin_ui_anim_control_scale_in_regist, plugin_ui_anim_control_scale_in_unregist }
    , { "ui-anim-control-scale-out", plugin_ui_anim_control_scale_out_regist, plugin_ui_anim_control_scale_out_unregist }
    , { "ui-anim-control-anim", plugin_ui_anim_control_anim_regist, plugin_ui_anim_control_anim_unregist }
    , { "ui-anim-control-move", plugin_ui_anim_control_move_regist, plugin_ui_anim_control_move_unregist }
    , { "ui-anim-control-scroll", plugin_ui_anim_control_scroll_regist, plugin_ui_anim_control_scroll_unregist }
    , { "ui-anim-control-frame-scale", plugin_ui_anim_control_frame_scale_regist, plugin_ui_anim_control_frame_scale_unregist }
    , { "ui-anim-control-frame-move", plugin_ui_anim_control_frame_move_regist, plugin_ui_anim_control_frame_move_unregist }
    , { "ui-anim-label-time-duration", plugin_ui_anim_label_time_duration_regist, plugin_ui_anim_label_time_duration_unregist }
    , { "ui-anim-progress-bind-control", plugin_ui_anim_progress_bind_control_regist, plugin_ui_anim_progress_bind_control_unregist }
    , { "ui-anim-frame-move", plugin_ui_anim_frame_move_regist, plugin_ui_anim_frame_move_unregist }
    , { "ui-anim-frame-alpha", plugin_ui_anim_frame_move_regist, plugin_ui_anim_frame_alpha_unregist }
    , { "ui-anim-frame-bind", plugin_ui_anim_frame_bind_regist, plugin_ui_anim_frame_bind_unregist }
    , { "ui-move-algorithm-linear", plugin_ui_move_algorithm_linear_regist, plugin_ui_move_algorithm_linear_unregist }
    , { "ui-move-algorithm-oval-quarter", plugin_ui_move_algorithm_oval_quarter_regist, plugin_ui_move_algorithm_oval_quarter_unregist }
    , { "ui-template-render", plugin_ui_template_render_regist, plugin_ui_template_render_unregist }
};

plugin_ui_module_t
plugin_ui_module_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr,
    plugin_package_module_t package_module,
    ui_runtime_module_t runtime,
    plugin_editor_module_t editor_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_ui_module * module;
    nm_node_t module_node;
    int8_t component_pos;

    assert(app);

    if (name == NULL) name = "plugin_ui_module";

    if (package_module == NULL) {
        CPE_ERROR(em, "%s: create: no package module", name);
        return NULL;
    }

    if (runtime == NULL) {
        CPE_ERROR(em, "%s: create: no package module", name);
        return NULL;
    }
    
    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_ui_module));
    if (module_node == NULL) return NULL;

    module = (plugin_ui_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_cfg_fps = 30.0f;
    module->m_data_mgr = data_mgr;
    module->m_package_module = package_module;
    module->m_runtime = runtime;
    module->m_editor_module = editor_module;

    module->m_animation_max_capacity = 0;
    module->m_animation_control_max_capacity = 0;
    module->m_move_algorithm_max_capacity = 0;
    
    mem_buffer_init(&module->m_dump_buffer, module->m_alloc);

    module->m_control_max_capacity = 0;
    module->m_control_metas = NULL;

    TAILQ_INIT(&module->m_envs);

    module->m_computer = xcomputer_create(alloc, em);
    if (module->m_computer == NULL) {
        nm_node_free(module_node);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &module->m_page_metas,
            alloc,
            (cpe_hash_fun_t) plugin_ui_page_meta_hash,
            (cpe_hash_eq_t) plugin_ui_page_meta_eq,
            CPE_HASH_OBJ2ENTRY(plugin_ui_page_meta, m_hh_for_module),
            -1) != 0)
    {
        mem_buffer_clear(&module->m_dump_buffer);
        xcomputer_free(module->m_computer);
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_animation_metas,
            alloc,
            (cpe_hash_fun_t) plugin_ui_animation_meta_hash,
            (cpe_hash_eq_t) plugin_ui_animation_meta_eq,
            CPE_HASH_OBJ2ENTRY(plugin_ui_animation_meta, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&module->m_page_metas);
        mem_buffer_clear(&module->m_dump_buffer);
        xcomputer_free(module->m_computer);
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_move_algorithm_metas,
            alloc,
            (cpe_hash_fun_t) plugin_ui_move_algorithm_meta_hash,
            (cpe_hash_eq_t) plugin_ui_move_algorithm_meta_eq,
            CPE_HASH_OBJ2ENTRY(plugin_ui_move_algorithm_meta, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&module->m_animation_metas);
        cpe_hash_table_fini(&module->m_page_metas);
        mem_buffer_clear(&module->m_dump_buffer);
        xcomputer_free(module->m_computer);
        nm_node_free(module_node);
        return NULL;
    }
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            cpe_hash_table_fini(&module->m_page_metas);
            cpe_hash_table_fini(&module->m_animation_metas);
            mem_buffer_clear(&module->m_dump_buffer);
            xcomputer_free(module->m_computer);
            nm_node_free(module_node);
            return NULL;
        }
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_plugin_ui_module);

    return module;
}

static void plugin_ui_module_clear(nm_node_t node) {
    plugin_ui_module_t module;
    int component_pos;

    module = nm_node_data(node);

    while(!TAILQ_EMPTY(&module->m_envs)) {
        plugin_ui_env_free(TAILQ_FIRST(&module->m_envs));
    }
    
    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }
    assert(module->m_control_metas == NULL);

    plugin_ui_page_meta_free_all(module);
    assert(cpe_hash_table_count(&module->m_page_metas) == 0);
    cpe_hash_table_fini(&module->m_page_metas);

    plugin_ui_animation_meta_free_all(module);
    assert(cpe_hash_table_count(&module->m_animation_metas) == 0);

    cpe_hash_table_fini(&module->m_animation_metas);

    plugin_ui_move_algorithm_meta_free_all(module);
    cpe_hash_table_fini(&module->m_move_algorithm_metas);
    
    mem_buffer_clear(&module->m_dump_buffer);

    xcomputer_free(module->m_computer);    
}

gd_app_context_t plugin_ui_module_app(plugin_ui_module_t module) {
    return module->m_app;
}

void plugin_ui_module_free(plugin_ui_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_ui_module) return;
    nm_node_free(module_node);
}

plugin_ui_module_t
plugin_ui_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_ui_module) return NULL;
    return (plugin_ui_module_t)nm_node_data(node);
}

plugin_ui_module_t
plugin_ui_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_ui_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_ui_module) return NULL;
    return (plugin_ui_module_t)nm_node_data(node);
}

const char * plugin_ui_module_name(plugin_ui_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

float plugin_ui_module_cfg_fps(plugin_ui_module_t mgr) {
    return mgr->m_cfg_fps;
}

plugin_editor_module_t plugin_ui_module_editor(plugin_ui_module_t mgr) {
    return mgr->m_editor_module;
}

uint32_t plugin_ui_module_control_base_size(plugin_ui_module_t mgr) {
    return sizeof(struct plugin_ui_control) + mgr->m_control_max_capacity;
}

uint32_t plugin_ui_module_animation_base_size(plugin_ui_module_t mgr) {
    return 0;
}

EXPORT_DIRECTIVE
int plugin_ui_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_ui_module_t plugin_ui_module;
    ui_data_mgr_t data_mgr;

    data_mgr = ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL));
    if (data_mgr == NULL) {
        APP_CTX_ERROR(app, "create plugin_ui_module: data-mgr not exist");
        return -1;
    }
    
    plugin_ui_module =
        plugin_ui_module_create(
            app, data_mgr,
            plugin_package_module_find_nc(app, NULL),
            ui_runtime_module_find_nc(app, NULL),
            plugin_editor_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_ui_module == NULL) return -1;

    plugin_ui_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_ui_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_ui_module_name(plugin_ui_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_ui_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_ui_module_t plugin_ui_module;

    plugin_ui_module = plugin_ui_module_find_nc(app, gd_app_module_name(module));
    if (plugin_ui_module) {
        plugin_ui_module_free(plugin_ui_module);
    }
}
