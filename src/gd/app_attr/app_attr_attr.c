#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "app_attr_attr_i.h"
#include "app_attr_provider_i.h"
#include "app_attr_request_i.h"
#include "app_attr_attr_binding_i.h"

app_attr_attr_t
app_attr_attr_create(app_attr_provider_t provider, const char * name, LPDRMETAENTRY entry, uint32_t start_pos) {
    app_attr_module_t module = provider->m_module;
    app_attr_attr_t attr;
    size_t name_len = strlen(name) + 1;
    
    attr = mem_calloc(module->m_alloc, sizeof(struct app_attr_attr) + name_len);
    if (attr == NULL) {
        CPE_ERROR(module->m_em, "app_attr: create attr %s: alloc fail!", dr_entry_name(entry));
        return NULL;
    }

    memcpy(attr + 1, name, name_len);
    attr->m_provider = provider;
    attr->m_name = (void *)(attr + 1);
    attr->m_entry = entry;
    attr->m_start_pos = start_pos;
    attr->m_state = app_attr_attr_readable;
    TAILQ_INIT(&attr->m_synchronizers);
    TAILQ_INIT(&attr->m_requests);
    
    cpe_hash_entry_init(&attr->m_hh);
    if (cpe_hash_table_insert_unique(&module->m_attrs, attr) != 0) {
        CPE_ERROR(module->m_em, "app_attr: attr %s insert fail!", name);
        mem_free(module->m_alloc, attr);
        return NULL;
    }
    
    TAILQ_INSERT_TAIL(&provider->m_attrs, attr, m_next_for_provider);

    return attr;
}

void app_attr_attr_free(app_attr_attr_t attr) {
    app_attr_module_t module = attr->m_provider->m_module;

    while(!TAILQ_EMPTY(&attr->m_synchronizers)) {
        app_attr_attr_binding_free(TAILQ_FIRST(&attr->m_synchronizers));
    }

    while(!TAILQ_EMPTY(&attr->m_requests)) {
        app_attr_attr_binding_free(TAILQ_FIRST(&attr->m_requests));
    }

    TAILQ_REMOVE(&attr->m_provider->m_attrs, attr, m_next_for_provider);

    cpe_hash_table_remove_by_ins(&module->m_attrs, attr);
    
    mem_free(module->m_alloc, attr);
}

app_attr_attr_t app_attr_attr_find(app_attr_module_t module, const char * name) {
    struct app_attr_attr key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_attrs, &key);
}

app_attr_attr_t app_attr_attr_find_in_provider(app_attr_provider_t provider, const char * name) {
    app_attr_attr_t attr;

    TAILQ_FOREACH(attr, &provider->m_attrs, m_next_for_provider) {
        if (strcmp(attr->m_name, name) == 0) return attr;
    }
    
    return NULL;
}

static app_attr_attr_t app_attr_provider_attr_next(struct app_attr_attr_it * it) {
    app_attr_attr_t * data = (app_attr_attr_t *)(it->m_data);
    app_attr_attr_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_provider);
    return r;
}

void app_attr_provider_attrs(app_attr_provider_t provider, app_attr_attr_it_t it) {
    *(app_attr_attr_t *)(it->m_data) = TAILQ_FIRST(&provider->m_attrs);
    it->next = app_attr_provider_attr_next;
}

static app_attr_attr_t app_attr_module_attr_next(struct app_attr_attr_it * it) {
    return cpe_hash_it_next((cpe_hash_it_t *)&it->m_data);
}

void app_attr_module_attrs(app_attr_module_t module, app_attr_attr_it_t it) {
    assert(sizeof(struct cpe_hash_it) <= CPE_ARRAY_SIZE(it->m_data));
    
    cpe_hash_it_init((cpe_hash_it_t *)&it->m_data, &module->m_attrs);
    it->next = app_attr_module_attr_next;
}

void app_attr_attr_set_readable(app_attr_attr_t attr) {
    app_attr_attr_binding_t attr_binding;
    
    if (attr->m_state ==  app_attr_attr_readable) return;
    
    attr->m_state = app_attr_attr_readable;
    TAILQ_FOREACH(attr_binding, &attr->m_requests, m_next_for_attr) {
        assert(attr_binding->m_type == app_attr_attr_binding_request);
        app_attr_request_set_state((app_attr_request_t)attr_binding->m_product, app_attr_request_check);
    }
}

uint32_t app_attr_attr_hash(app_attr_attr_t attr) {
    return cpe_hash_str(attr->m_name, strlen(attr->m_name));
}

int app_attr_attr_eq(app_attr_attr_t l, app_attr_attr_t r) {
    return strcmp(l->m_name, r->m_name) == 0 ? 1 : 0;
}
