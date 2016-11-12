#ifndef SVR_SET_SVR_BINDING_H
#define SVR_SET_SVR_BINDING_H
#include "set_svr_set.h"
#include "set_svr_svr_type.h"

struct set_svr_svr_ins {
    set_svr_svr_type_t m_svr_type;
    uint16_t m_svr_id;
    set_svr_set_t m_set;
    set_chanel_t m_chanel;

    TAILQ_ENTRY(set_svr_svr_ins) m_next_for_svr_type;
    union {
        TAILQ_ENTRY(set_svr_svr_ins) m_next_for_set;
        TAILQ_ENTRY(set_svr_svr_ins) m_next_for_local;
    };

    struct cpe_hash_entry m_hh;
};

/*operations of set_svr_ins*/
set_svr_svr_ins_t set_svr_svr_ins_create(set_svr_svr_type_t type, uint16_t svr_id, set_svr_set_t set);
set_svr_svr_ins_t set_svr_svr_ins_find(set_svr_t svr, uint16_t svr_type_id, uint16_t svr_id);
void set_svr_svr_ins_free(set_svr_svr_ins_t svr_ins);
void set_svr_svr_ins_free_all(set_svr_t svr);

uint32_t set_svr_svr_ins_hash(set_svr_svr_ins_t o);
int set_svr_svr_ins_eq(set_svr_svr_ins_t l, set_svr_svr_ins_t r);

#endif
