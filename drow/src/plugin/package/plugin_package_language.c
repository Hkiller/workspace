#include <assert.h>
#include "render/model/ui_data_src_group.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_language.h"
#include "render/cache/ui_cache_group.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_package_language_i.h"

plugin_package_language_t
plugin_package_language_create(plugin_package_package_t package, ui_data_language_t data_language) {
    plugin_package_module_t module = package->m_module;
    plugin_package_language_t package_language;

    package_language = mem_alloc(module->m_alloc, sizeof(struct plugin_package_language));
    if (package_language == NULL) {
        CPE_ERROR(module->m_em, "plugin_package_language_create: alloc fail!");
        return NULL;
    }

    package_language->m_package = package;
    package_language->m_data_language = data_language;

    package_language->m_resources = ui_cache_group_create(module->m_cache_mgr);
    if (package_language->m_resources == NULL) {
        CPE_ERROR(module->m_em, "plugin_package_language_create: create resources fail!");
        mem_free(module->m_alloc, package_language);
        return NULL;
    }
    
    package_language->m_srcs = ui_data_src_group_create(module->m_data_mgr);
    if (package_language->m_srcs == NULL) {
        CPE_ERROR(module->m_em, "plugin_package_language_create: create src fail!");
        ui_cache_group_free(package_language->m_resources);
        mem_free(module->m_alloc, package_language);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&package->m_languages, package_language, m_next_for_package);

    return package_language;
}

void plugin_package_language_free(plugin_package_language_t package_language) {
    plugin_package_module_t module = package_language->m_package->m_module;

    if (package_language->m_package->m_active_language == package_language) {
        package_language->m_package->m_active_language = NULL;
    }
    
    ui_cache_group_free(package_language->m_resources);
    package_language->m_resources = NULL;

    ui_data_src_group_free(package_language->m_srcs);
    package_language->m_srcs = NULL;

    TAILQ_REMOVE(&package_language->m_package->m_languages, package_language, m_next_for_package);
    
    mem_free(module->m_alloc, package_language);
}

plugin_package_language_t
plugin_package_language_find(plugin_package_package_t package, ui_data_language_t data_language) {
    plugin_package_language_t package_language;

    TAILQ_FOREACH(package_language, &package->m_languages, m_next_for_package) {
        if (package_language->m_data_language == data_language) return package_language;
    }

    return NULL;
}

ui_cache_group_t plugin_package_language_resources(plugin_package_language_t package_language) {
    return package_language->m_resources;
}

ui_data_src_group_t plugin_package_language_srcs(plugin_package_language_t package_language) {
    return package_language->m_srcs;
}
