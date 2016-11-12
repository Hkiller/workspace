#ifndef SVR_CENTER_SVR_INS_PROXY_H
#define SVR_CENTER_SVR_INS_PROXY_H
#include "center_svr_conn.h"

struct center_svr_ins_proxy {
    center_svr_t m_svr;
    center_svr_type_t m_svr_type;
    center_svr_set_proxy_t m_set;
    SVR_CENTER_CLI_RECORD * m_data;

    TAILQ_ENTRY(center_svr_ins_proxy) m_next_for_type;
    TAILQ_ENTRY(center_svr_ins_proxy) m_next_for_set;

    struct cpe_hash_entry m_hh;
};

center_svr_ins_proxy_t center_svr_ins_proxy_create(
    center_svr_t svr, center_svr_type_t svr_type, center_svr_set_proxy_t set,
    SVR_CENTER_CLI_RECORD * data);
center_svr_ins_proxy_t center_svr_ins_proxy_find(center_svr_t svr, uint16_t svr_type, uint16_t svr_id);
void center_svr_ins_proxy_free(center_svr_ins_proxy_t cli);
void center_svr_ins_proxy_free_all(center_svr_t svr);

uint32_t center_svr_ins_proxy_hash(center_svr_ins_proxy_t cli);
int center_svr_ins_proxy_eq(center_svr_ins_proxy_t l, center_svr_ins_proxy_t r);

#endif
