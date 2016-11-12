#ifndef SVR_SET_SVR_SVR_TYPE_H
#define SVR_SET_SVR_SVR_TYPE_H
#include "set_svr.h"

struct set_svr_svr_type {
    set_svr_t m_svr;

    uint16_t m_svr_type_id;
    char * m_svr_type_name;
    set_svr_scope_t m_svr_scope;

    LPDRMETA m_pkg_meta;

    set_svr_svr_ins_list_t m_runing_inses;

    struct cpe_hash_entry m_hh_by_id;
    struct cpe_hash_entry m_hh_by_name;
};

/*operations of set_svr_svr_type*/
set_svr_svr_type_t set_svr_svr_type_create(set_svr_t svr, const char * svr_type, cfg_t cfg);
void set_svr_svr_type_free(set_svr_svr_type_t svr_type);
void set_svr_svr_type_free_all(set_svr_t svr);

set_svr_svr_type_t set_svr_svr_type_find_by_id(set_svr_t svr, uint16_t svr_type_id);
set_svr_svr_type_t set_svr_svr_type_find_by_name(set_svr_t svr, const char * svr_type_name);

uint32_t set_svr_svr_type_hash_by_id(set_svr_svr_type_t o);
int set_svr_svr_type_eq_by_id(set_svr_svr_type_t l, set_svr_svr_type_t r);

uint32_t set_svr_svr_type_hash_by_name(set_svr_svr_type_t o);
int set_svr_svr_type_eq_by_name(set_svr_svr_type_t l, set_svr_svr_type_t r);

int set_svr_set_pkg_meta(set_svr_t svr, dp_req_t body, set_svr_svr_type_t to_svr_type, set_svr_svr_type_t from_svr_type);

#endif
