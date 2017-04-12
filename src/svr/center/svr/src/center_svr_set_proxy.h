#ifndef SVR_CENTER_SVR_SET_PROXY_H
#define SVR_CENTER_SVR_SET_PROXY_H
#include "center_svr_conn.h"

struct center_svr_set_proxy {
    center_svr_t m_svr;
    SVR_CENTER_SET * m_set;
    uint16_t m_ins_count;
    center_svr_ins_proxy_list_t m_ins_proxies;
    uint32_t m_offline_time;
    center_svr_conn_t m_conn;
    struct cpe_hash_entry m_hh;
};

center_svr_set_proxy_t center_svr_set_proxy_create(center_svr_t svr, SVR_CENTER_SET const * set_info);
center_svr_set_proxy_t center_svr_set_proxy_find(center_svr_t svr, uint16_t set_id);
void center_svr_set_proxy_free(center_svr_set_proxy_t set);
void center_svr_set_proxy_free_all(center_svr_t svr);

void center_svr_set_proxy_set_conn(center_svr_set_proxy_t set, center_svr_conn_t conn);

uint8_t center_svr_set_proxy_is_match(center_svr_set_proxy_t set, SVR_CENTER_SET const * set_info);
void center_svr_set_proxy_update(center_svr_set_proxy_t set, SVR_CENTER_SET const * set_info);

uint32_t center_svr_set_proxy_hash(center_svr_set_proxy_t set);
int center_svr_set_proxy_eq(center_svr_set_proxy_t l, center_svr_set_proxy_t r);

#endif
