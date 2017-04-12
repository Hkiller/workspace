#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "plugin_ui_control_attr_meta_i.h"
#include "plugin_ui_control_meta_i.h"
#include "plugin_ui_module_i.h"

plugin_ui_control_attr_meta_t
plugin_ui_control_attr_meta_create(
    plugin_ui_control_meta_t control_meta, const char * attr_name,
    plugin_ui_control_attr_set_fun_t setter,
    plugin_ui_control_attr_get_fun_t getter)
{
    plugin_ui_module_t module = control_meta->m_module;
    plugin_ui_control_attr_meta_t attr_meta;

    attr_meta = mem_alloc(module->m_alloc, sizeof(struct plugin_ui_control_attr_meta));
    if (attr_meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_control_attr_meta_create: alloc fail!");
        return NULL;
    }

    attr_meta->m_control_meta = control_meta;
    cpe_str_dup(attr_meta->m_attr_name, sizeof(attr_meta->m_attr_name), attr_name);
    attr_meta->m_setter = setter;
    attr_meta->m_getter = getter;
    
    cpe_hash_entry_init(&attr_meta->m_hh);
    if (cpe_hash_table_insert(&control_meta->m_attr_metas, attr_meta) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_control_attr_meta_create: name %s duplicate!", attr_name);
        mem_free(module->m_alloc, attr_meta);
        return NULL;
    }

    return attr_meta;
}
    
void plugin_ui_control_attr_meta_free(plugin_ui_control_attr_meta_t attr_meta) {
    cpe_hash_table_remove_by_ins(&attr_meta->m_control_meta->m_attr_metas, attr_meta);
    mem_free(attr_meta->m_control_meta->m_module->m_alloc, attr_meta);
}

plugin_ui_control_attr_meta_t
plugin_ui_control_attr_meta_find(plugin_ui_control_meta_t control_meta, const char * attr_name) {
    struct plugin_ui_control_attr_meta key;
    cpe_str_dup(key.m_attr_name, sizeof(key.m_attr_name), attr_name);
    return cpe_hash_table_find(&control_meta->m_attr_metas, &key);
}

void plugin_ui_control_attr_meta_free_all(plugin_ui_control_meta_t meta) {
    struct cpe_hash_it control_attr_meta_it;
    plugin_ui_control_attr_meta_t control_attr_meta;

    cpe_hash_it_init(&control_attr_meta_it, &meta->m_attr_metas);

    control_attr_meta = cpe_hash_it_next(&control_attr_meta_it);
    while (control_attr_meta) {
        plugin_ui_control_attr_meta_t next = cpe_hash_it_next(&control_attr_meta_it);
        plugin_ui_control_attr_meta_free(control_attr_meta);
        control_attr_meta = next;
    }
}

uint32_t plugin_ui_control_attr_meta_hash(const plugin_ui_control_attr_meta_t attr_meta) {
    return cpe_hash_str(attr_meta->m_attr_name, strlen(attr_meta->m_attr_name));
}

int plugin_ui_control_attr_meta_eq(const plugin_ui_control_attr_meta_t l, const plugin_ui_control_attr_meta_t r) {
    return strcmp(l->m_attr_name, r->m_attr_name) == 0;
}
