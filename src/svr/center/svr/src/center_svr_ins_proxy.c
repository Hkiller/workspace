#include <assert.h> 
#include "cpe/pal/pal_string.h"
#include "center_svr_ins_proxy.h"
#include "center_svr_set_proxy.h"
#include "center_svr_type.h"

center_svr_ins_proxy_t
center_svr_ins_proxy_create(center_svr_t svr, center_svr_type_t svr_type, center_svr_set_proxy_t set, SVR_CENTER_CLI_RECORD * record) {
    center_svr_ins_proxy_t data;

    data = mem_alloc(svr->m_alloc, sizeof(struct center_svr_ins_proxy));
    if (data == NULL) {
        CPE_ERROR(svr->m_em, "%s: create data: malloc fail!", center_svr_name(svr));
        return NULL;
    }

    data->m_svr = svr;
    data->m_set = set;
    data->m_data = record;
    data->m_svr_type = svr_type;

    cpe_hash_entry_init(&data->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_ins_proxies, data) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: create data: insert fail, %d.%d already exist!",
            center_svr_name(svr), record->svr_type, record->svr_id);
        mem_free(svr->m_alloc, data);
        return NULL;
    }
    
    svr_type->m_svr_count++;
    TAILQ_INSERT_TAIL(&svr_type->m_ins_proxies, data, m_next_for_type);

    set->m_ins_count++;
    TAILQ_INSERT_TAIL(&set->m_ins_proxies, data, m_next_for_set);

    return data;
}

void center_svr_ins_proxy_free(center_svr_ins_proxy_t ins) {
    center_svr_t svr = ins->m_svr;

    assert(svr);

    /*clear data*/
    assert(ins->m_data);
    ins->m_data = NULL;

    /*disconnect with connection */
    assert(ins->m_set);
    assert(ins->m_set->m_ins_count > 0);
    ins->m_set->m_ins_count--;
    TAILQ_REMOVE(&ins->m_set->m_ins_proxies, ins, m_next_for_set);
    ins->m_set = NULL;

    /*remove from svr_type*/
    assert(ins->m_svr_type);
    ins->m_svr_type->m_svr_count--;
    TAILQ_REMOVE(&ins->m_svr_type->m_ins_proxies, ins, m_next_for_type);
    ins->m_svr_type = NULL;

    /*remove from svr*/
    cpe_hash_table_remove_by_ins(&svr->m_ins_proxies, ins);

    mem_free(svr->m_alloc, ins);
}

void center_svr_ins_proxy_free_all(center_svr_t svr) {
    struct cpe_hash_it data_it;
    center_svr_ins_proxy_t data;

    cpe_hash_it_init(&data_it, &svr->m_ins_proxies);

    data = cpe_hash_it_next(&data_it);
    while(data) {
        center_svr_ins_proxy_t next = cpe_hash_it_next(&data_it);
        center_svr_ins_proxy_free(data);
        data = next;
    }
}

center_svr_ins_proxy_t
center_svr_ins_proxy_find(center_svr_t svr, uint16_t svr_type, uint16_t svr_id) {
    SVR_CENTER_CLI_RECORD key_data;
    struct center_svr_ins_proxy key;

    key.m_data = &key_data;
    key_data.svr_type = svr_type;
    key_data.svr_id = svr_id;

    return cpe_hash_table_find(&svr->m_ins_proxies, &key);
}

uint32_t center_svr_ins_proxy_hash(center_svr_ins_proxy_t ins) {
    return ((uint32_t)ins->m_data->svr_type) << 16
        | ins->m_data->svr_id;
}

int center_svr_ins_proxy_eq(center_svr_ins_proxy_t l, center_svr_ins_proxy_t r) {
    return (l->m_data->svr_type == r->m_data->svr_type
            && l->m_data->svr_id == r->m_data->svr_id) ? 1 : 0;
}

