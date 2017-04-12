#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "appsvr_umeng_click_info.h"

appsvr_umeng_click_info_t
appsvr_umeng_click_info_create(
    appsvr_umeng_module_t module, const char * page_name, const char * control_name,
    const char * id, const char * value, const char * attrs)
{
    appsvr_umeng_click_info_t click_info;
    size_t page_name_len = strlen(page_name) + 1;
    size_t control_name_len = strlen(control_name) + 1;
    size_t id_len = strlen(id) + 1;
    size_t value_len = strlen(value) + 1;
    size_t attrs_len = attrs ? strlen(attrs) + 1 : 0;
    char * p;
    
    click_info = mem_alloc(
        module->m_alloc, sizeof(struct appsvr_umeng_click_info) + page_name_len + control_name_len + id_len + value_len + attrs_len);
    if (click_info == NULL) {
        CPE_ERROR(module->m_em, "appsvr_umeng_click_info_create: alloc fail!");
        return NULL;
    }

    click_info->m_module = module;

    p = (void*)(click_info + 1);
    memcpy(p, page_name, page_name_len);
    *cpe_str_trim_tail(p + page_name_len - 1, p) = 0;
    click_info->m_page_name = p;
    p += page_name_len;
    
    memcpy(p, control_name, control_name_len);
    *cpe_str_trim_tail(p + control_name_len - 1, p) = 0;
    click_info->m_control_name = p;
    p += control_name_len;
    
    memcpy(p, id, id_len);
    *cpe_str_trim_tail(p + id_len - 1, p) = 0;
    click_info->m_id = p;
    p += id_len;

    memcpy(p, value, value_len);
    *cpe_str_trim_tail(p + value_len - 1, p) = 0;
    click_info->m_value = p;
    p += value_len;
    
    if (attrs) {
        memcpy(p, attrs, attrs_len);
        *cpe_str_trim_tail(p + attrs_len - 1, p) = 0;
        click_info->m_attrs = p;
        p += attrs_len;
    }
    else {
        click_info->m_attrs = NULL;
    }
    
    cpe_hash_entry_init(&click_info->m_hh);
    if (cpe_hash_table_insert(&module->m_click_infos, click_info) != 0) {
        CPE_ERROR(module->m_em, "appsvr_umeng_click_info_create: %s.%s insert fail!", page_name, control_name);
        mem_free(module->m_alloc, click_info);
        return NULL;
    }
    
    return click_info;
}

void appsvr_umeng_click_info_free(appsvr_umeng_click_info_t click_info) {
    appsvr_umeng_module_t module = click_info->m_module;

    cpe_hash_table_remove_by_ins(&module->m_click_infos, click_info);

    mem_free(module->m_alloc, click_info);
}

void appsvr_umeng_click_info_free_all(appsvr_umeng_module_t module) {
    struct cpe_hash_it click_info_it;
    appsvr_umeng_click_info_t click_info;

    cpe_hash_it_init(&click_info_it, &module->m_click_infos);

    click_info = cpe_hash_it_next(&click_info_it);
    while (click_info) {
        appsvr_umeng_click_info_t next = cpe_hash_it_next(&click_info_it);
        appsvr_umeng_click_info_free(click_info);
        click_info = next;
    }
}

uint32_t appsvr_umeng_click_info_hash(appsvr_umeng_click_info_t click_info) {
    return cpe_hash_str(click_info->m_control_name, strlen(click_info->m_control_name));
}

int appsvr_umeng_click_info_eq(appsvr_umeng_click_info_t l, appsvr_umeng_click_info_t r) {
    int rv = strcmp(l->m_control_name, r->m_control_name);
    if (rv) return 0;

    rv = strcmp(l->m_page_name, r->m_page_name);
    return rv == 0 ? 1 : 0;
}

int appsvr_umeng_load_click_infos(appsvr_umeng_module_t module, cfg_t cfg) {
    struct cfg_it data_it;
    cfg_t data_cfg;
    
    cfg_it_init(&data_it, cfg_find_cfg(cfg, "click"));
    while((data_cfg = cfg_it_next(&data_it))) {
        const char * control = cfg_get_string(data_cfg, "control", NULL);
        const char * id = cfg_get_string(data_cfg, "id", NULL);
        const char * value = cfg_get_string(data_cfg, "value", NULL);
        const char * attrs = cfg_get_string(data_cfg, "attrs", NULL);
        char buf[32];
        const char * sep;
        const char * page_name;
        
        if (control == NULL) {
            CPE_ERROR(module->m_em, "umeng: load check: control not configured");
            return -1;
        }

        if (id == NULL) {
            CPE_ERROR(module->m_em, "umeng: load check: id not configured");
            return -1;
        }

        if (value == NULL) {
            CPE_ERROR(module->m_em, "umeng: load check: value not configured");
            return -1;
        }

        sep = strchr(control, '.');
        if (sep == NULL) {
            CPE_ERROR(module->m_em, "umeng: load check: control %s format error", control);
            return -1;
        }

        page_name = cpe_str_dup_range(buf, sizeof(buf), control, sep);
        if (page_name == NULL) {
            CPE_ERROR(module->m_em, "umeng: load check: control %s read page fail", control);
            return -1;

        }

        if (appsvr_umeng_click_info_create(module, page_name, sep + 1, id, value, attrs) == NULL) {
            CPE_ERROR(module->m_em, "umeng: load check: create click info fail");
            return -1;
        }
    }

    return 0;
}
