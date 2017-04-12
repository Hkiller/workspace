#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "app_attr_attr_binding_i.h"
#include "app_attr_synchronizer_i.h"
#include "app_attr_request_i.h"

app_attr_attr_binding_t
app_attr_attr_binding_create(
    app_attr_attr_binding_type_t binding_type, app_attr_attr_t attr, void * product)
{
    app_attr_module_t module = attr->m_provider->m_module;
    app_attr_attr_binding_t attr_binding;
    
    attr_binding = TAILQ_FIRST(&module->m_free_attr_bindings);
    if (attr_binding) {
        TAILQ_REMOVE(&module->m_free_attr_bindings, attr_binding, m_next_for_attr);
    }
    else {
        attr_binding = mem_alloc(module->m_alloc, sizeof(struct app_attr_attr_binding));
        if (attr_binding == NULL) {
            CPE_ERROR(module->m_em, "app_attr_attr_binding_create: alloc fail!");
            return NULL;
        }
    }

    attr_binding->m_type = binding_type;
    attr_binding->m_attr = attr;
    attr_binding->m_product = product;

    switch(binding_type) {
    case app_attr_attr_binding_synchronizer:
        TAILQ_INSERT_TAIL(&attr->m_synchronizers, attr_binding, m_next_for_attr);
        TAILQ_INSERT_TAIL(&((app_attr_synchronizer_t)product)->m_attrs, attr_binding, m_next_for_product);
        break;
    case app_attr_attr_binding_request:
        TAILQ_INSERT_TAIL(&attr->m_requests, attr_binding, m_next_for_attr);
        TAILQ_INSERT_TAIL(&((app_attr_request_t)product)->m_attrs, attr_binding, m_next_for_product);
        break;
    default:
        CPE_ERROR(module->m_em, "app_attr_attr_binding_create: binding_type %d unknown!", binding_type);
        attr_binding->m_attr = (app_attr_attr_t)module;
        TAILQ_INSERT_TAIL(&module->m_free_attr_bindings, attr_binding, m_next_for_attr);
        return NULL;
    }

    return attr_binding;
}

void app_attr_attr_binding_free(app_attr_attr_binding_t attr_binding) {
    app_attr_module_t module = attr_binding->m_attr->m_provider->m_module;

    switch(attr_binding->m_type) {
    case app_attr_attr_binding_synchronizer:
        TAILQ_REMOVE(&attr_binding->m_attr->m_synchronizers, attr_binding, m_next_for_attr);
        TAILQ_REMOVE(&((app_attr_synchronizer_t)attr_binding->m_product)->m_attrs, attr_binding, m_next_for_product);
        break;
    case app_attr_attr_binding_request:
        TAILQ_REMOVE(&attr_binding->m_attr->m_requests, attr_binding, m_next_for_attr);
        TAILQ_REMOVE(&((app_attr_request_t)attr_binding->m_product)->m_attrs, attr_binding, m_next_for_product);
        break;
    default:
        assert(0);
    }
    
    attr_binding->m_attr = (app_attr_attr_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_attr_bindings, attr_binding, m_next_for_attr);
}

void app_attr_attr_binding_real_free(app_attr_attr_binding_t attr_binding) {
    app_attr_module_t module = (app_attr_module_t)attr_binding->m_attr;
    TAILQ_REMOVE(&module->m_free_attr_bindings, attr_binding, m_next_for_attr);
    mem_free(module->m_alloc, attr_binding);
}
