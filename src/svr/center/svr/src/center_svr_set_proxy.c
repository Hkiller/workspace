#include <assert.h> 
#include "cpe/pal/pal_string.h"
#include "center_svr_set_proxy.h"
#include "center_svr_ins_proxy.h"

center_svr_set_proxy_t
center_svr_set_proxy_create(center_svr_t svr, SVR_CENTER_SET const * set_info) {
    center_svr_set_proxy_t set;

    set = mem_alloc(svr->m_alloc, sizeof(struct center_svr_set_proxy) + sizeof(SVR_CENTER_SET));
    if (set == NULL) {
        CPE_ERROR(svr->m_em, "%s: create set: malloc fail!", center_svr_name(svr));
        return NULL;
    }

    set->m_svr = svr;
    set->m_set = (void*)(set + 1);
    set->m_conn = NULL;
    set->m_offline_time = center_svr_cur_time(svr);
    set->m_ins_count = 0;
    TAILQ_INIT(&set->m_ins_proxies);
    memcpy(set->m_set, set_info, sizeof(*set_info));
    
    cpe_hash_entry_init(&set->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_set_proxies, set) != 0) {
        CPE_ERROR(svr->m_em, "%s: create set proxy: set %d already exist!", center_svr_name(svr), set_info->id);
        mem_free(svr->m_alloc, set);
        return NULL;
    }
    
    return set;
}

void center_svr_set_proxy_free(center_svr_set_proxy_t set) {
    center_svr_t svr = set->m_svr;

    assert(svr);

    /*disconnect with connection */
    if (set->m_conn) {
        center_svr_set_proxy_set_conn(set, NULL);
        assert(set->m_conn == NULL);
    }

    while(!TAILQ_EMPTY(&set->m_ins_proxies)) {
        center_svr_ins_proxy_free(TAILQ_FIRST(&set->m_ins_proxies));
    }
    assert(set->m_ins_count == 0);
    
    /*remove from svr*/
    cpe_hash_table_remove_by_ins(&svr->m_set_proxies, set);

    mem_free(svr->m_alloc, set);
}

void center_svr_set_proxy_free_all(center_svr_t svr) {
    struct cpe_hash_it data_it;
    center_svr_set_proxy_t data;

    cpe_hash_it_init(&data_it, &svr->m_set_proxies);

    data = cpe_hash_it_next(&data_it);
    while(data) {
        center_svr_set_proxy_t next = cpe_hash_it_next(&data_it);
        center_svr_set_proxy_free(data);
        data = next;
    }
}

center_svr_set_proxy_t
center_svr_set_proxy_find(center_svr_t svr, uint16_t set_id) {
    SVR_CENTER_SET key_data;
    struct center_svr_set_proxy key;
    
    key_data.id = set_id;
    key.m_set = &key_data;
    
    return cpe_hash_table_find(&svr->m_set_proxies, &key);
}

void center_svr_set_proxy_set_conn(center_svr_set_proxy_t set, center_svr_conn_t conn) {
    if (set->m_conn) {
        if (set->m_svr->m_debug) {
            CPE_INFO(
                set->m_svr->m_em, "%s: set %d: close old conn %d!",
                center_svr_name(set->m_svr), set->m_set->id, set->m_conn->m_fd);
        }
        assert(set->m_conn->m_set == set);
        center_svr_conn_free(set->m_conn);
        assert(set->m_conn == NULL);
    }

    set->m_conn = conn;

    if (set->m_conn) {
        set->m_conn->m_set = set;
    }
    else {
        set->m_offline_time = center_svr_cur_time(set->m_svr);
    }
}

uint8_t center_svr_set_proxy_is_match(center_svr_set_proxy_t set, SVR_CENTER_SET const * set_info) {
    assert(set->m_set->id == set_info->id);
    return (set->m_set->region == set_info->region
            && set->m_set->port == set_info->port
            && strcmp(set->m_set->ip, set_info->ip) == 0)
        ? 1 : 0;
}

void center_svr_set_proxy_update(center_svr_set_proxy_t set, SVR_CENTER_SET const * set_info) {
    center_svr_ins_proxy_t ins;
    
    assert(set->m_set->id == set_info->id);

    if (set->m_svr->m_debug) {
        CPE_INFO(
            set->m_svr->m_em, "%s: set %d update: region=%d,ip=%s,port=%d ==> region=%d,ip=%s,port=%d!",
            center_svr_name(set->m_svr), set->m_set->id,
            set->m_set->region, set->m_set->ip, set->m_set->port,
            set_info->region, set_info->ip, set_info->port);
    }

    memcpy(set->m_set, set_info, sizeof(*set_info));

    TAILQ_FOREACH(ins, &set->m_ins_proxies, m_next_for_type) {
        ins->m_data->set = *set_info;
    }
}

uint32_t center_svr_set_proxy_hash(center_svr_set_proxy_t set) {
    return set->m_set->id;
}

int center_svr_set_proxy_eq(center_svr_set_proxy_t l, center_svr_set_proxy_t r) {
    return (l->m_set->id == r->m_set->id) ? 1 : 0;
}
