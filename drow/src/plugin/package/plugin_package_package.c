#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_group.h"
#include "plugin_package_package_i.h"
#include "plugin_package_queue_i.h"
#include "plugin_package_language_i.h"
#include "plugin_package_group_ref_i.h"
#include "plugin_package_depend_i.h"

plugin_package_package_t
plugin_package_package_create(plugin_package_module_t module, const char * name, plugin_package_package_state_t state) {
    plugin_package_package_t package;
    size_t name_len = strlen(name) + 1;
    
    package = mem_alloc(module->m_alloc, sizeof(struct plugin_package_package) + name_len);
    if (package == NULL) {
        CPE_ERROR(module->m_em, "plugin_package_package_create: package create fail!");
        return NULL;
    }

    memcpy(package + 1, name, name_len);

    package->m_module = module;
    package->m_state = plugin_package_package_empty;
    package->m_name = (void*)(package + 1);
    package->m_path = NULL;
    package->m_resources = ui_cache_group_create(module->m_cache_mgr);
    package->m_srcs = ui_data_src_group_create(module->m_data_mgr);
    TAILQ_INIT(&package->m_groups);
    TAILQ_INIT(&package->m_languages);
    TAILQ_INIT(&package->m_base_packages);
    TAILQ_INIT(&package->m_extern_packages);
    package->m_active_language = NULL;
    package->m_progress = 0.0f;
    package->m_total_size = 0;
    package->m_loading_res_group = NULL;
    package->m_loading_src_group = NULL;

    cpe_hash_entry_init(&package->m_hh);
    if (cpe_hash_table_insert_unique(&module->m_packages, package) != 0) {
        CPE_ERROR(module->m_em, "plugin_package_package_create: package %s duplicate!", name);
        return NULL;
    }

    plugin_package_package_set_state(package, state);
    
    return package;
}

void plugin_package_package_free(plugin_package_package_t package) {
    plugin_package_module_t module = package->m_module;

    switch(package->m_state) {
    case plugin_package_package_loading:
    case plugin_package_package_loaded:
        plugin_package_package_unload(package);
        assert(package->m_state == plugin_package_package_installed);
        break;
    default:
        break;
    }
    assert(package->m_active_language == NULL);
    plugin_package_package_set_state(package, plugin_package_package_empty);

    while(!TAILQ_EMPTY(&package->m_languages)) {
        plugin_package_language_free(TAILQ_FIRST(&package->m_languages));
    }
    
    ui_cache_group_free(package->m_resources);
    ui_data_src_group_free(package->m_srcs);
    
    if (package->m_loading_res_group) {
        ui_cache_group_free(package->m_loading_res_group);
        package->m_loading_res_group = NULL;
    }
    
    if (package->m_loading_src_group) {
        ui_data_src_group_free(package->m_loading_src_group);
        package->m_loading_src_group = NULL;
    }
    
    if (package->m_path) {
        mem_free(module->m_alloc, package->m_path);
        package->m_path = NULL;
    }

    while(!TAILQ_EMPTY(&package->m_groups)) {
        plugin_package_group_ref_free(TAILQ_FIRST(&package->m_groups));
    }
    
    while(!TAILQ_EMPTY(&package->m_base_packages)) {
        plugin_package_depend_free(TAILQ_FIRST(&package->m_base_packages));
    }

    while(!TAILQ_EMPTY(&package->m_extern_packages)) {
        plugin_package_depend_free(TAILQ_FIRST(&package->m_extern_packages));
    }
    
    cpe_hash_table_remove_by_ins(&module->m_packages, package);

    mem_free(module->m_alloc, package);
}

void plugin_package_package_free_all(plugin_package_module_t module) {
    struct cpe_hash_it package_it;
    plugin_package_package_t package;

    cpe_hash_it_init(&package_it, &module->m_packages);

    package = cpe_hash_it_next(&package_it);
    while (package) {
        plugin_package_package_t next = cpe_hash_it_next(&package_it);
        plugin_package_package_free(package);
        package = next;
    }
}

plugin_package_package_t plugin_package_package_find(plugin_package_module_t module, const char * name) {
    struct plugin_package_package key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_packages, &key);
}

plugin_package_module_t plugin_package_package_module(plugin_package_package_t package) {
    return package->m_module;
}
    
uint8_t plugin_package_package_is_in_group(plugin_package_package_t package, plugin_package_group_t group) {
    plugin_package_group_ref_t ref;

    TAILQ_FOREACH(ref, &package->m_groups, m_next_for_package) {
        if (ref->m_group == group) return 1;
    }

    return 0;
}

plugin_package_package_using_state_t plugin_package_package_using_state(plugin_package_package_t package) {
    plugin_package_group_ref_t ref;
    plugin_package_package_using_state_t using_state = plugin_package_package_using_state_free;
    
    TAILQ_FOREACH(ref, &package->m_groups, m_next_for_package) {
        if (ref->m_group->m_package_using_state > using_state) {
            using_state = ref->m_group->m_package_using_state;
        }
    }

    return using_state;
}

plugin_package_package_state_t plugin_package_package_state(plugin_package_package_t package) {
    return package->m_state;
}

const char * plugin_package_package_state_str(plugin_package_package_t package) {
    return plugin_package_package_state_to_str(package->m_state);
}

const char * plugin_package_package_name(plugin_package_package_t package) {
    return package->m_name;
}

const char * plugin_package_package_path(plugin_package_package_t package) {
    return package->m_path;
}

int plugin_package_package_set_path(plugin_package_package_t package, const char * path) {
    char * new_path = NULL;

    if (path) {
        new_path = cpe_str_mem_dup(package->m_module->m_alloc, path);
        if (new_path == NULL) {
            CPE_ERROR(package->m_module->m_em, "package %s: set path alloc fail!", plugin_package_package_name(package));
            return -1;
        }
    }
    
    switch(package->m_state) {
    case plugin_package_package_empty:
        break;
    case plugin_package_package_downloading:
        if (package->m_progress != 1.0f) {
            CPE_ERROR(
                package->m_module->m_em, "package %s: state is %s, but not complete, can`t set path",
                plugin_package_package_name(package), plugin_package_package_state_str(package));
            return -1;
        }
        package->m_progress = 0.0f;
        break;
    default:
        CPE_ERROR(
            package->m_module->m_em, "package %s: state is %s, can`t set path",
            plugin_package_package_name(package), plugin_package_package_state_str(package));
        return -1;
    }
    
    if (package->m_path) mem_free(package->m_module->m_alloc, package->m_path);
    package->m_path = new_path;
    plugin_package_package_set_state(package, plugin_package_package_empty);
    
    return 0;
}

ui_cache_group_t plugin_package_package_resources(plugin_package_package_t package) {
    return package->m_resources;
}

ui_data_src_group_t plugin_package_package_srcs(plugin_package_package_t package) {
    return package->m_srcs;
}

uint8_t plugin_package_package_has_base_package(plugin_package_package_t package, plugin_package_package_t base_package) {
    plugin_package_depend_t depend;

    TAILQ_FOREACH(depend, &package->m_base_packages, m_next_for_extern) {
        if (depend->m_base_package == base_package) return 1;
    }

    return 0;
}

int plugin_package_package_add_base_package(plugin_package_package_t package, plugin_package_package_t base_package) {
    if (plugin_package_package_has_base_package(package, base_package)) return 0;

    if (plugin_package_depend_create(base_package, package) == NULL) return -1;
    
    return 0;
}

uint32_t plugin_package_package_hash(plugin_package_package_t package) {
    return cpe_hash_str(package->m_name, strlen(package->m_name));
}

int plugin_package_package_eq(plugin_package_package_t l, plugin_package_package_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

static plugin_package_package_t plugin_package_module_package_it_do_next(plugin_package_package_it_t it) {
    struct cpe_hash_it * hash_it = (void*)it->m_data;
    return cpe_hash_it_next(hash_it);    
}

void plugin_package_module_packages(plugin_package_module_t module, plugin_package_package_it_t package_it) {
    struct cpe_hash_it * hash_it = (void*)package_it->m_data;
    
    assert(CPE_ARRAY_SIZE(package_it->m_data) >= sizeof(struct cpe_hash_it));

    cpe_hash_it_init(hash_it, &module->m_packages);
    package_it->next = plugin_package_module_package_it_do_next;
}

static plugin_package_package_t plugin_package_package_state_package_next(struct plugin_package_package_it * package_it) {
    plugin_package_package_t * data  = (plugin_package_package_t *)package_it->m_data;
    plugin_package_package_t r = *data;

    if (r) {
        *data = TAILQ_NEXT(r, m_next_for_module);
    }

    return r;
}
    
void plugin_package_module_loaded_packages(plugin_package_module_t module, plugin_package_package_it_t package_it) {
    *(plugin_package_package_t *)package_it->m_data = TAILQ_FIRST(&module->m_loaded_packages);
    package_it->next = plugin_package_package_state_package_next;
}

void plugin_package_module_downloading_packages(plugin_package_module_t module, plugin_package_package_it_t package_it) {
    *(plugin_package_package_t *)package_it->m_data = TAILQ_FIRST(&module->m_downloading_packages);
    package_it->next = plugin_package_package_state_package_next;
}

void plugin_package_module_loading_packages(plugin_package_module_t module, plugin_package_package_it_t package_it) {
    *(plugin_package_package_t *)package_it->m_data = TAILQ_FIRST(&module->m_loading_packages);
    package_it->next = plugin_package_package_state_package_next;
}

static plugin_package_package_t plugin_package_package_base_package_next(struct plugin_package_package_it * it) {
    plugin_package_depend_t * data = (plugin_package_depend_t *)(it->m_data);
    plugin_package_depend_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_extern);
    return r->m_base_package;
}

void plugin_package_package_base_packages(plugin_package_package_t package, plugin_package_package_it_t it) {
    plugin_package_depend_t * data = (plugin_package_depend_t *)(it->m_data);
    *data = TAILQ_FIRST(&package->m_base_packages);
    it->next = plugin_package_package_base_package_next;
}

static plugin_package_package_t plugin_package_package_extern_package_next(struct plugin_package_package_it * it) {
    plugin_package_depend_t * data = (plugin_package_depend_t *)(it->m_data);
    plugin_package_depend_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_base);
    return r->m_extern_package;
}

void plugin_package_package_extern_packages(plugin_package_package_t package, plugin_package_package_it_t it) {
    plugin_package_depend_t * data = (plugin_package_depend_t *)(it->m_data);
    *data = TAILQ_FIRST(&package->m_extern_packages);
    it->next = plugin_package_package_extern_package_next;
}

uint32_t plugin_package_package_total_size(plugin_package_package_t package) {
    return package->m_total_size;
}

void plugin_package_package_set_total_size(plugin_package_package_t package, uint32_t size) {
    package->m_total_size = size;
}

float plugin_package_package_progress(plugin_package_package_t package) {
    return package->m_progress;
}

int plugin_package_package_set_progress(plugin_package_package_t package, float progress) {
    if (progress < 1.0f || progress > 1.0f) {
        CPE_ERROR(package->m_module->m_em, "plugin_package_package_set_progress: progress %f range error!", progress);
        return -1;
    }
    
    if (package->m_state == plugin_package_package_downloading) {
        package->m_progress = progress;

        /*下载进度由外部设置，触发状态切换 */
        if (package->m_progress == 0.0f) {
            plugin_package_package_set_state(package, plugin_package_package_empty);
        }
    }
    else if (package->m_state == plugin_package_package_loading) {
        package->m_progress = progress;
        /*loading进度内部触发，不需要切换状态 */
    }
    else {
        CPE_ERROR(
            package->m_module->m_em,
            "plugin_package_package_set_progress: package %s is in state %s, can`t set progress!",
            plugin_package_package_name(package),
            plugin_package_package_state_str(package));
        return -1;
    }
    
    return 0;
}

void plugin_package_package_set_state(plugin_package_package_t package, plugin_package_package_state_t state) {
    plugin_package_module_t module = package->m_module;
    
    if (package->m_state == state) return;
    
    switch(package->m_state) {
    case plugin_package_package_downloading:
        assert(module->m_downloading_package_count > 0);
        module->m_downloading_package_count--;
        TAILQ_REMOVE(&module->m_downloading_packages, package, m_next_for_module);
        break;
    case plugin_package_package_loading:
        assert(module->m_loading_package_count > 0);
        module->m_loading_package_count--;
        TAILQ_REMOVE(&module->m_loading_packages, package, m_next_for_module);
        break;
    case plugin_package_package_loaded:
        assert(module->m_loaded_package_count > 0);
        module->m_loaded_package_count--;
        TAILQ_REMOVE(&module->m_loaded_packages, package, m_next_for_module);
        break;
    default:
        break;
    }

    package->m_progress = 0.0f;
    package->m_state = state;
        
    switch(package->m_state) {
    case plugin_package_package_downloading:
        module->m_downloading_package_count++;
        TAILQ_INSERT_TAIL(&package->m_module->m_downloading_packages, package, m_next_for_module);
        break;
    case plugin_package_package_loading:
        module->m_loading_package_count++;
        TAILQ_INSERT_TAIL(&package->m_module->m_loading_packages, package, m_next_for_module);
        break;
    case plugin_package_package_loaded:
        module->m_loaded_package_count++;
        TAILQ_INSERT_TAIL(&package->m_module->m_loaded_packages, package, m_next_for_module);
        break;
    default:
        break;
    }
}

const char * plugin_package_package_dump_using(plugin_package_package_t package, mem_buffer_t buffer) {
    plugin_package_group_ref_t ref;

    mem_buffer_clear_data(buffer);
    mem_buffer_printf(buffer, "%s: ", plugin_package_package_name(package));
    TAILQ_FOREACH(ref, &package->m_groups, m_next_for_package) {
        if (ref->m_group->m_package_using_state == plugin_package_package_using_state_free) continue;
        mem_buffer_strcat(buffer, ref->m_group->m_name);
        mem_buffer_strcat(buffer, ",");
    }
    mem_buffer_append_char(buffer, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

uint8_t plugin_package_package_is_installed(plugin_package_package_t package) {
    return package->m_state != plugin_package_package_empty
        && package->m_state != plugin_package_package_downloading;
}

const char * plugin_package_package_state_to_str(plugin_package_package_state_t state) {
    switch(state) {
    case plugin_package_package_empty:
        return "empty";
    case plugin_package_package_downloading:
        return "downloading";
    case plugin_package_package_installed:
        return "installed";
    case plugin_package_package_loading:
        return "loading";
    case plugin_package_package_loaded:
        return "loaded";
    default:
        return "unknown";
    }
}
