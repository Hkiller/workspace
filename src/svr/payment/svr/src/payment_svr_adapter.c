#include "cpe/cfg/cfg_read.h"
#include "payment_svr_adapter.h"
#include "payment_svr_adapter_type.h"

payment_svr_adapter_t
payment_svr_adapter_create(payment_svr_t svr, payment_svr_adapter_type_t type, cfg_t cfg) {
    payment_svr_adapter_t adapter;

    adapter = mem_calloc(svr->m_alloc, sizeof(struct payment_svr_adapter));
    if (adapter == NULL) {
        CPE_ERROR(svr->m_em, "payment_svr_adapter_create: %s: alloc fail!", type->m_service_name);
        return NULL;
    }

    adapter->m_svr = svr;
    adapter->m_type = type;

    if (type->m_init(adapter, cfg) != 0) {
        CPE_ERROR(svr->m_em, "payment_svr_adapter_create: %s: init fail!", type->m_service_name);
        mem_free(svr->m_alloc, adapter);
        return NULL;
    }
    
    TAILQ_INSERT_TAIL(&svr->m_adapters, adapter, m_next);
    return adapter;
}

void payment_svr_adapter_free(payment_svr_adapter_t adapter) {
    payment_svr_t svr = adapter->m_svr;
    
    adapter->m_type->m_fini(adapter);

    TAILQ_REMOVE(&svr->m_adapters, adapter, m_next);
    
    mem_free(adapter->m_svr->m_alloc, adapter);
}

payment_svr_adapter_t payment_svr_adapter_find_by_type_id(payment_svr_t svr, uint8_t type_id) {
    payment_svr_adapter_t adapter;

    TAILQ_FOREACH(adapter, &svr->m_adapters, m_next) {
        if (adapter->m_type->m_service_type == type_id) return adapter;
    }

    return NULL;
}

int payment_svr_adapter_load(payment_svr_t svr, cfg_t cfg) {
    struct cfg_it adapter_cfg_it;
    cfg_t adapter_cfg;
    
    cfg_it_init(&adapter_cfg_it, cfg);
    while((adapter_cfg = cfg_it_next(&adapter_cfg_it))) {
        const char * type_name = cfg_name(adapter_cfg);
        payment_svr_adapter_type_t adapter_type = NULL;
        uint8_t i;

        for(i = 0; i < g_adapter_type_count; ++i) {
            payment_svr_adapter_type_t check = &g_adapter_types[i];
            if (strcmp(check->m_service_name, type_name) == 0) {
                adapter_type = check;
                break;
            }
        }

        if (adapter_type == NULL) {
            CPE_ERROR(svr->m_em, "payment_svr_adapter_load: adapter type %s not exist!", type_name);
            return -1;
        }

        if (payment_svr_adapter_create(svr, adapter_type, adapter_cfg) == NULL) {
            CPE_ERROR(svr->m_em, "payment_svr_adapter_load: adapter create fail, type=%s!", type_name);
            return -1;
        }
    }
    
    return 0;
}
