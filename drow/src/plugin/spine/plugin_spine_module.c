#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/cache/ui_cache_manager.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "plugin_spine_module_i.h"
#include "plugin_spine_data_skeleton_i.h"
#include "plugin_spine_data_state_def_i.h"
#include "plugin_spine_track_listener_i.h"
#include "plugin_spine_obj_part_i.h"
#include "plugin_spine_obj_track_i.h"
#include "plugin_spine_obj_anim_i.h"
#include "plugin_spine_obj_anim_group_binding_i.h"
#include "plugin_spine_obj_ik_i.h"

extern char g_metalib_plugin_spine[];
static void plugin_spine_module_do_clear(nm_node_t node);

#define PLUGIN_SPINE_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_plugin_spine, __name); \
    assert(module-> __arg)

struct nm_node_type s_nm_node_type_plugin_spine_module = {
    "plugin_spine_module",
    plugin_spine_module_do_clear
};

plugin_spine_module_t
plugin_spine_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    const char * name, error_monitor_t em)
{
    struct plugin_spine_module * module;
    nm_node_t module_node;

    assert(app);

    if (name == NULL) name = "plugin_spine_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_spine_module));
    if (module_node == NULL) return NULL;

    module = (plugin_spine_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;

    module->m_data_mgr = data_mgr;
    module->m_cache_mgr = cache_mgr;
    module->m_runtime = runtime;

    module->m_free_listeners = NULL;
    TAILQ_INIT(&module->m_free_parts);
    TAILQ_INIT(&module->m_free_part_states);
    TAILQ_INIT(&module->m_free_part_transitions);
    TAILQ_INIT(&module->m_free_tracks);
    TAILQ_INIT(&module->m_free_anims);
    TAILQ_INIT(&module->m_free_anim_group_bindings);
    TAILQ_INIT(&module->m_free_iks);

    PLUGIN_SPINE_MODULE_LOAD_META(m_meta_data_part, "spine_part");
    PLUGIN_SPINE_MODULE_LOAD_META(m_meta_data_part_state, "spine_part_state");
    PLUGIN_SPINE_MODULE_LOAD_META(m_meta_data_part_transition, "spine_part_transition");

    if (ui_data_mgr_register_type(
            data_mgr, ui_data_src_type_spine_skeleton,
            (ui_data_product_create_fun_t)plugin_spine_data_skeleton_create, module,
            (ui_data_product_free_fun_t)plugin_spine_data_skeleton_free, module,
            plugin_spine_data_skeleton_update_usings)
        != 0)
    {
        CPE_ERROR(em, "%s: create: register type spine skeleton fail!", name);
        nm_node_free(module_node);
        return NULL;
    }

    if (ui_data_mgr_register_type(
            data_mgr, ui_data_src_type_spine_state_def,
            (ui_data_product_create_fun_t)plugin_spine_data_state_def_create, module,
            (ui_data_product_free_fun_t)plugin_spine_data_state_def_free, module,
            NULL)
        != 0)
    {
        CPE_ERROR(em, "%s: create: register type spine state_def fail!", name);
        ui_data_mgr_unregister_type(data_mgr, ui_data_src_type_spine_skeleton);
        nm_node_free(module_node);
        return NULL;
    }
    
    if (runtime) {
        ui_runtime_render_obj_meta_t obj_meta;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                runtime, "skeleton", UI_OBJECT_TYPE_SKELETON, sizeof(struct plugin_spine_obj), module,
                plugin_spine_obj_do_init,
                plugin_spine_obj_do_set,
                plugin_spine_obj_do_setup,
                plugin_spine_obj_do_update,
                plugin_spine_obj_do_free,
                plugin_spine_obj_do_render,
                plugin_spine_obj_do_is_playing,
                NULL,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(em, "%s: create: register render obj fail", name);
            ui_data_mgr_unregister_type(data_mgr, ui_data_src_type_spine_state_def);
            ui_data_mgr_unregister_type(data_mgr, ui_data_src_type_spine_skeleton);
            nm_node_free(module_node);
            return NULL;
        }

    }

    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_spine_skeleton, plugin_spine_data_skeleton_bin_load, module);

    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_spine_state_def, plugin_spine_data_state_def_bin_load, module);
    
    ui_data_mgr_set_saver(
        module->m_data_mgr, ui_data_src_type_spine_state_def,
        plugin_spine_data_state_def_bin_save, plugin_spine_data_state_def_bin_rm, module);
    
    mem_buffer_init(&module->m_dump_buffer, alloc);
    
    nm_node_set_type(module_node, &s_nm_node_type_plugin_spine_module);

    return module;
}

static void plugin_spine_module_do_clear(nm_node_t node) {
    plugin_spine_module_t module;

    module = nm_node_data(node);


    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_id(module->m_runtime, UI_OBJECT_TYPE_SKELETON);
        if (obj_meta) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
    
    ui_data_mgr_unregister_type(module->m_data_mgr, ui_data_src_type_spine_skeleton);
    ui_data_mgr_unregister_type(module->m_data_mgr, ui_data_src_type_spine_state_def);
    
    plugin_spine_track_listener_real_free_all(module);
    plugin_spine_obj_part_real_free_all(module);
    plugin_spine_obj_part_state_real_free_all(module);
    plugin_spine_obj_part_transition_real_free_all(module);
    plugin_spine_obj_track_real_free_all(module);
    plugin_spine_obj_anim_real_free_all(module);
    plugin_spine_obj_anim_group_binding_real_free_all(module);
    plugin_spine_obj_ik_real_free_all(module);
    
    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t plugin_spine_module_app(plugin_spine_module_t module) {
    return module->m_app;
}

void plugin_spine_module_free(plugin_spine_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_spine_module) return;
    nm_node_free(module_node);
}

plugin_spine_module_t
plugin_spine_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_spine_module) return NULL;
    return (plugin_spine_module_t)nm_node_data(node);
}

plugin_spine_module_t
plugin_spine_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_spine_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_spine_module) return NULL;
    return (plugin_spine_module_t)nm_node_data(node);
}

const char * plugin_spine_module_name(plugin_spine_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

ui_data_mgr_t plugin_spine_module_data_mgr(plugin_spine_module_t module) {
    return module->m_data_mgr;
}

ui_runtime_module_t plugin_spine_module_runtime(plugin_spine_module_t module) {
    return module->m_runtime;
}

LPDRMETA plugin_spine_module_meta_data_part(plugin_spine_module_t module) {
    return module->m_meta_data_part;
}

LPDRMETA plugin_spine_module_meta_data_part_state(plugin_spine_module_t module) {
    return module->m_meta_data_part_state;
}

LPDRMETA plugin_spine_module_meta_data_part_transition(plugin_spine_module_t module) {
    return module->m_meta_data_part_transition;
}

void plugin_spine_module_install_bin_loader(plugin_spine_module_t module) {
    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_spine_skeleton, plugin_spine_data_skeleton_bin_load, module);

    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_spine_state_def, plugin_spine_data_state_def_bin_load, module);
}

void plugin_spine_module_install_bin_saver(plugin_spine_module_t module) {
    ui_data_mgr_set_saver(
        module->m_data_mgr, ui_data_src_type_spine_state_def,
        plugin_spine_data_state_def_bin_save, plugin_spine_data_state_def_bin_rm, module);
}

