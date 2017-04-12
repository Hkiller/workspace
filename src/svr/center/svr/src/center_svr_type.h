#ifndef SVR_CENTER_SVR_TYPE_H
#define SVR_CENTER_SVR_TYPE_H
#include "center_svr.h"

/*组网服务组（就是服务类型）*/
struct center_svr_type {
    center_svr_t m_svr;
    char * m_svr_type_name;
    uint16_t m_svr_type;
    uint16_t m_svr_count;

    center_svr_ins_proxy_list_t m_ins_proxies;         /*服务实例列表*/

    struct cpe_hash_entry m_hh;
};

center_svr_type_t center_svr_type_create(center_svr_t svr, const char * svr_type_name, uint16_t svr_type);
center_svr_type_t center_svr_type_find(center_svr_t svr, uint16_t svr_type);
center_svr_type_t center_svr_type_lsearch_by_name(center_svr_t svr, const char * svr_type_name);
void center_svr_type_free(center_svr_type_t svr_type);
void center_svr_type_free_all(center_svr_t svr);

uint32_t center_svr_type_hash(center_svr_type_t svr_type);
int center_svr_type_eq(center_svr_type_t l, center_svr_type_t r);

#endif
