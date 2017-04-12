#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "plugin_package_module_i.h"
#include "plugin_package_load_task_i.h"
#include "plugin_package_package_i.h"
#include "plugin_package_installer_i.h"
#include "plugin_package_queue_i.h"
#include "plugin_package_group_i.h"
#include "plugin_package_group_ref_i.h"
#include "plugin_package_region_i.h"

static void plugin_package_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_package_module = {
    "plugin_package_module",
    plugin_package_module_clear
};

plugin_package_module_t
plugin_package_module_create(
    gd_app_context_t app,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    plugin_package_module_t package_module;
    nm_node_t package_module_node;

    assert(app);

    if (name == NULL) name = "plugin_package_module";

    package_module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_package_module));
    if (package_module_node == NULL) return NULL;

    package_module = (plugin_package_module_t)nm_node_data(package_module_node);

    package_module->m_app = app;
    package_module->m_alloc = alloc;
    package_module->m_em = em;
    package_module->m_debug = 0;
    package_module->m_is_control_res = 1;
    package_module->m_data_mgr = data_mgr;
    package_module->m_cache_mgr = cache_mgr;
    package_module->m_repo_path = NULL;
    package_module->m_installer = NULL;
    package_module->m_to_download_packages = NULL;
    package_module->m_to_load_packages = NULL;
    package_module->m_downloading_package_count = 0;
    package_module->m_loading_package_count = 0;
    package_module->m_loaded_package_count = 0;
    package_module->m_max_load_task_id = 0;
    TAILQ_INIT(&package_module->m_downloading_packages);
    TAILQ_INIT(&package_module->m_loading_packages);
    TAILQ_INIT(&package_module->m_loaded_packages);
    TAILQ_INIT(&package_module->m_groups);
    TAILQ_INIT(&package_module->m_load_tasks);
    TAILQ_INIT(&package_module->m_regions);

    TAILQ_INIT(&package_module->m_free_load_tasks);
    TAILQ_INIT(&package_module->m_free_groups);
    TAILQ_INIT(&package_module->m_free_group_refs);

    package_module->m_downloading_package_limit = 1;
    package_module->m_loading_package_limit = 1;
    package_module->m_process_tick_limit_ms = 1000 / 120;
    package_module->m_total_load_complete_count = 0;
    package_module->m_total_load_count = 0;
    package_module->m_total_download_complete_count = 0;
    package_module->m_total_download_count = 0;
    
    if (gd_app_tick_add(package_module->m_app, plugin_package_module_tick, package_module, 0) != 0) {
        CPE_ERROR(package_module->m_em, "plugin_package_module: add tick fail!");
        nm_node_free(package_module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &package_module->m_packages,
            alloc,
            (cpe_hash_fun_t) plugin_package_package_hash,
            (cpe_hash_eq_t) plugin_package_package_eq,
            CPE_HASH_OBJ2ENTRY(plugin_package_package, m_hh),
            -1) != 0)
    {
        CPE_ERROR(package_module->m_em, "plugin_package_module: init hash table!");
        gd_app_tick_remove(package_module->m_app, plugin_package_module_tick, package_module);
        nm_node_free(package_module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &package_module->m_queues,
            alloc,
            (cpe_hash_fun_t) plugin_package_queue_hash,
            (cpe_hash_eq_t) plugin_package_queue_eq,
            CPE_HASH_OBJ2ENTRY(plugin_package_queue, m_hh),
            -1) != 0)
    {
        CPE_ERROR(package_module->m_em, "plugin_package_module: init hash table!");
        cpe_hash_table_fini(&package_module->m_packages);
        gd_app_tick_remove(package_module->m_app, plugin_package_module_tick, package_module);
        nm_node_free(package_module_node);
        return NULL;
    }

    nm_node_set_type(package_module_node, &s_nm_node_type_plugin_package_module);

    return package_module;
}

static void plugin_package_module_clear(nm_node_t node) {
    plugin_package_module_t package_module;

    package_module = nm_node_data(node);

    if(package_module->m_installer) {
        plugin_package_installer_free(package_module->m_installer);
        assert(package_module->m_installer == NULL);
    }
    
    gd_app_tick_remove(package_module->m_app, plugin_package_module_tick, package_module);
    
    plugin_package_package_free_all(package_module);
    cpe_hash_table_fini(&package_module->m_packages);
    assert(package_module->m_downloading_package_count == 0);
    assert(package_module->m_loading_package_count == 0);
    assert(package_module->m_loaded_package_count == 0);
    assert(TAILQ_EMPTY(&package_module->m_downloading_packages));
    assert(TAILQ_EMPTY(&package_module->m_loading_packages));
    assert(TAILQ_EMPTY(&package_module->m_loaded_packages));

    plugin_package_queue_free_all(package_module);
    cpe_hash_table_fini(&package_module->m_queues);

    while(!TAILQ_EMPTY(&package_module->m_regions)) {
        plugin_package_region_free(TAILQ_FIRST(&package_module->m_regions));
    }

    if (package_module->m_to_download_packages) {
        plugin_package_group_free(package_module->m_to_download_packages);
        package_module->m_to_download_packages = NULL;
    }
    
    if (package_module->m_to_load_packages) {
        plugin_package_group_free(package_module->m_to_load_packages);
        package_module->m_to_load_packages = NULL;
    }

    while(!TAILQ_EMPTY(&package_module->m_load_tasks)) {
        plugin_package_load_task_free(TAILQ_FIRST(&package_module->m_load_tasks));
    }
    
    while(!TAILQ_EMPTY(&package_module->m_groups)) {
        plugin_package_group_free(TAILQ_FIRST(&package_module->m_groups));
    }
    
    while(!TAILQ_EMPTY(&package_module->m_free_groups)) {
        plugin_package_group_real_free(TAILQ_FIRST(&package_module->m_free_groups));
    }

    while(!TAILQ_EMPTY(&package_module->m_free_load_tasks)) {
        plugin_package_load_task_real_free(TAILQ_FIRST(&package_module->m_free_load_tasks));
    }
    
    while(!TAILQ_EMPTY(&package_module->m_free_group_refs)) {
        plugin_package_group_ref_real_free(TAILQ_FIRST(&package_module->m_free_group_refs));
    }

    if (package_module->m_repo_path) {
        mem_free(package_module->m_alloc, package_module->m_repo_path);
        package_module->m_repo_path = NULL;
    }
}

gd_app_context_t plugin_package_module_app(plugin_package_module_t package_module) {
    return package_module->m_app;
}

void plugin_package_module_free(plugin_package_module_t package_module) {
    nm_node_t package_module_node;
    assert(package_module);

    package_module_node = nm_node_from_data(package_module);
    if (nm_node_type(package_module_node) != &s_nm_node_type_plugin_package_module) return;
    nm_node_free(package_module_node);
}

plugin_package_module_t
plugin_package_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_package_module) return NULL;
    return (plugin_package_module_t)nm_node_data(node);
}

plugin_package_module_t
plugin_package_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_package_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_package_module) return NULL;
    return (plugin_package_module_t)nm_node_data(node);
}

const char * plugin_package_module_name(plugin_package_module_t package_module) {
    return nm_node_name(nm_node_from_data(package_module));
}

ui_data_mgr_t plugin_package_module_data_mgr(plugin_package_module_t package_module) {
    return package_module->m_data_mgr;
}

ui_cache_manager_t plugin_package_module_cache_mgr(plugin_package_module_t package_module) {
    return package_module->m_cache_mgr;
}

uint16_t plugin_package_module_downloading_package_limit(plugin_package_module_t module) {
    return module->m_downloading_package_limit;
}

void plugin_package_module_set_downloading_package_limit(plugin_package_module_t module, uint16_t limit) {
    module->m_downloading_package_limit = limit;
}

uint16_t plugin_package_module_loading_package_limit(plugin_package_module_t module) {
    return module->m_loading_package_limit;
}

void plugin_package_module_set_loading_package_limit(plugin_package_module_t module, uint16_t limit) {
    module->m_loading_package_limit = limit;
}
    
uint16_t plugin_package_module_process_tick_limit_ms(plugin_package_module_t module) {
    return module->m_process_tick_limit_ms;
}

void plugin_package_module_set_process_tick_limit_ms(plugin_package_module_t module, uint16_t limit) {
    module->m_process_tick_limit_ms = limit;
}

uint8_t plugin_package_module_is_control_res(plugin_package_module_t module) {
    return module->m_is_control_res;
}

void plugin_package_module_set_control_res(plugin_package_module_t module, uint8_t control_res) {
    assert(cpe_hash_table_count(&module->m_packages) == 0);
    module->m_is_control_res = control_res;
}

const char * plugin_package_module_repo_path(plugin_package_module_t module) {
    return module->m_repo_path;
}

int plugin_package_module_set_repo_path(plugin_package_module_t module, const char * path) {
    if (module->m_repo_path) {
        mem_free(module->m_alloc, module->m_repo_path);
    }

    if (path) {
        module->m_repo_path = cpe_str_mem_dup(module->m_alloc, path);
        if (module->m_repo_path == NULL) {
            CPE_ERROR(module->m_em, "plugin_package_module_set_repo_path: dup str fail!");
            return -1;
        }
    }
    else {
        module->m_repo_path = NULL;
    }
    
    return 0;
}

void plugin_package_module_total_reset(plugin_package_module_t module) {
    plugin_package_group_ref_t ref;
    
    module->m_total_download_count = 0;
    module->m_total_download_complete_count = 0;
    module->m_total_load_count = 0;
    module->m_total_load_complete_count = 0;

    /*处理需要下载的包 */
    if (module->m_to_download_packages) {
        TAILQ_FOREACH(ref,&module->m_to_download_packages->m_packages, m_next_for_group) {
            switch(ref->m_package->m_state) {
            case plugin_package_package_empty:
            case plugin_package_package_downloading:
                module->m_total_download_count++;
                module->m_total_load_count++;
                break;
            case plugin_package_package_installed:
            case plugin_package_package_loading:
            case plugin_package_package_loaded:
            default:
                module->m_total_load_count++;
                break;
            }
        }
    }

    if (module->m_to_load_packages) {
        TAILQ_FOREACH(ref,&module->m_to_load_packages->m_packages, m_next_for_group) {
            module->m_total_load_count++;
        }
    }
}

uint16_t plugin_package_module_downloading_count(plugin_package_module_t module) {
    return module->m_loading_package_count;
}

uint16_t plugin_package_module_loading_count(plugin_package_module_t module) {
    return module->m_loading_package_count;
}

uint16_t plugin_package_module_total_download_complete_count(plugin_package_module_t module) {
    return module->m_total_download_complete_count;
}

uint16_t plugin_package_module_total_download_count(plugin_package_module_t module) {
    return module->m_total_download_count;
}

uint16_t plugin_package_module_total_load_complete_count(plugin_package_module_t module) {
    return module->m_total_load_complete_count;
}

uint16_t plugin_package_module_total_load_count(plugin_package_module_t module) {
    return module->m_total_load_count;
}

void plugin_package_module_gc(plugin_package_module_t module) {
    plugin_package_package_t package, next_package;

    if (module->m_debug) {
        CPE_INFO(module->m_em, "==== begin package gc");
    }
    
    for(package = TAILQ_FIRST(&module->m_loading_packages); package; package = next_package) {
        next_package = TAILQ_NEXT(package, m_next_for_module);

        if (plugin_package_package_using_state(package) < plugin_package_package_using_state_ref_count) {
            plugin_package_package_unload(package);
        }
        else {
            if (module->m_debug) {
                CPE_INFO(module->m_em, "    %s", plugin_package_package_dump_using(package, gd_app_tmp_buffer(module->m_app)));
            }
        }
    }

    for(package = TAILQ_FIRST(&module->m_loaded_packages); package; package = next_package) {
        next_package = TAILQ_NEXT(package, m_next_for_module);

        if (plugin_package_package_using_state(package) < plugin_package_package_using_state_ref_count) {
            plugin_package_package_unload(package);
        }
        else {
            if (module->m_debug) {
                CPE_INFO(module->m_em, "    %s", plugin_package_package_dump_using(package, gd_app_tmp_buffer(module->m_app)));
            }
        }
    }
    
    if (module->m_debug) {
        CPE_INFO(module->m_em, "==== end package gc");
    }
}

void plugin_package_module_reinstall(plugin_package_module_t module) {
    plugin_package_package_t package;
    
    TAILQ_FOREACH(package, &module->m_loaded_packages, m_next_for_module) {
        struct ui_cache_res_it res_it;
        ui_cache_res_t res;

        ui_cache_group_using_resources(&res_it, package->m_resources);
        while((res = ui_cache_res_it_next(&res_it))) {
            if (ui_cache_res_type(res) != ui_cache_res_type_texture) continue;
            if (ui_cache_res_load_state(res) != ui_cache_res_not_load) continue;

            if (ui_cache_res_load_async(res, package->m_path) != 0) {
                CPE_ERROR(
                    module->m_em, "reinstall: package %s: res %s reload async fail!",
                    package->m_name, ui_cache_res_path(res));
                continue;
            }

            CPE_INFO(module->m_em, "reinstall: package %s: res %s", package->m_name, ui_cache_res_path(res));
        }
    }
}

EXPORT_DIRECTIVE
int plugin_package_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_package_module_t plugin_package_module;
    ui_data_mgr_t data_mgr;
    
    data_mgr = ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL));
    if (data_mgr == NULL) {
        APP_CTX_ERROR(app, "create %s: data-mgr not exist", gd_app_module_name(module));
        return -1;
    }

    plugin_package_module =
        plugin_package_module_create(
            app, data_mgr,
            ui_cache_manager_find_nc(app, cfg_get_string(cfg, "cache-mgr", NULL)),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_package_module == NULL) return -1;

    plugin_package_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_package_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_package_module_name(plugin_package_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_package_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_package_module_t plugin_package_module;

    plugin_package_module = plugin_package_module_find_nc(app, gd_app_module_name(module));
    if (plugin_package_module) {
        plugin_package_module_free(plugin_package_module);
    }
}
