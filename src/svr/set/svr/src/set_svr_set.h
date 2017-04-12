#ifndef SVR_SET_SVR_ROUTER_TYPES_H
#define SVR_SET_SVR_ROUTER_TYPES_H
#include "set_svr.h"

struct set_svr_set {
    set_svr_t m_svr;

    uint16_t m_set_id;
    uint16_t m_region;
    char m_ip[32];
    uint16_t m_port;
    
    set_svr_set_conn_t m_conn;
    set_svr_svr_ins_list_t m_svr_inses;

    struct cpe_hash_entry m_hh_by_id;
};

LPDRMETA set_svr_get_pkg_meta(set_svr_t svr, dp_req_t head, set_svr_svr_type_t to_svr_type, set_svr_svr_type_t from_svr_type);

/*operations of set_svr_set*/
set_svr_set_t set_svr_set_create(set_svr_t svr, uint16_t id, uint16_t region, const char * ip, uint16_t port);
void set_svr_set_free(set_svr_set_t svr_set);
void set_svr_set_free_all(set_svr_t svr);

void set_svr_set_set_region(set_svr_set_t set, uint16_t region);
void set_svr_set_set_target(set_svr_set_t set, const char * ip, uint16_t port);

set_svr_set_t set_svr_set_find_by_id(set_svr_t svr, uint32_t id);

uint32_t set_svr_set_hash_by_id(set_svr_set_t o);
int set_svr_set_eq_by_id(set_svr_set_t l, set_svr_set_t r);

const char * set_svr_set_name(set_svr_set_t set);
    
#endif
