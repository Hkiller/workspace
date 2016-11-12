#include "svr/set/share/set_repository.h"
#include "set_svr_svr_ins.h"

set_svr_svr_ins_t set_svr_svr_ins_create(set_svr_svr_type_t type, uint16_t svr_id, set_svr_set_t set) {
    set_svr_t svr = type->m_svr;
    set_svr_svr_ins_t svr_ins;

    svr_ins = mem_alloc(svr->m_alloc, sizeof(struct set_svr_svr_ins));
    if (svr_ins == NULL) {
        CPE_ERROR(svr->m_em, "%s: create svr ins: malloc fail!", set_svr_name(svr));
        return NULL;
    }

    svr_ins->m_svr_type = type;
    svr_ins->m_svr_id = svr_id;
    svr_ins->m_set = set;
    svr_ins->m_chanel = NULL;

    cpe_hash_entry_init(&svr_ins->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_svr_inses, svr_ins) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: svr %d.%d: insert fail, %d.%d already exist!",
            set_svr_name(svr), type->m_svr_type_id, svr_ins->m_svr_id,
            type->m_svr_type_id, svr_ins->m_svr_id);
        mem_free(svr->m_alloc, svr_ins);
        return NULL;
    }

    if (set == NULL) {
        ++svr->m_local_svr_count;
        TAILQ_INSERT_TAIL(&svr->m_local_svrs, svr_ins, m_next_for_local);
    }
    else {
        TAILQ_INSERT_TAIL(&set->m_svr_inses, svr_ins, m_next_for_set);
    }
    
    TAILQ_INSERT_TAIL(&type->m_runing_inses, svr_ins, m_next_for_svr_type);

    return svr_ins;
}

void set_svr_svr_ins_free(set_svr_svr_ins_t svr_ins) {
    set_svr_t svr = svr_ins->m_svr_type->m_svr;

    if (svr_ins->m_chanel) {
        set_repository_chanel_detach(svr_ins->m_chanel, svr->m_em);
        svr_ins->m_chanel = NULL;
    }
    
    if (svr_ins->m_set == NULL) {
        --svr->m_local_svr_count;
        TAILQ_REMOVE(&svr->m_local_svrs, svr_ins, m_next_for_local);
    }
    else {
        TAILQ_REMOVE(&svr_ins->m_set->m_svr_inses, svr_ins, m_next_for_set);
    }
    
    TAILQ_REMOVE(&svr_ins->m_svr_type->m_runing_inses, svr_ins, m_next_for_svr_type);

    cpe_hash_table_remove_by_ins(&svr->m_svr_inses, svr_ins);

    mem_free(svr->m_alloc, svr_ins);
}

void set_svr_svr_ins_free_all(set_svr_t svr) {
    struct cpe_hash_it data_it;
    set_svr_svr_ins_t data;

    cpe_hash_it_init(&data_it, &svr->m_svr_inses);

    data = cpe_hash_it_next(&data_it);
    while(data) {
        set_svr_svr_ins_t next = cpe_hash_it_next(&data_it);
        set_svr_svr_ins_free(data);
        data = next;
    }
}

set_svr_svr_ins_t set_svr_svr_ins_find(set_svr_t svr, uint16_t svr_type_id, uint16_t svr_id) {
    struct set_svr_svr_type key_type;
    struct set_svr_svr_ins key;

    key_type.m_svr_type_id = svr_type_id;
    key.m_svr_id = svr_id;
    key.m_svr_type = &key_type;

    return cpe_hash_table_find(&svr->m_svr_inses, &key);
}

uint32_t set_svr_svr_ins_hash(set_svr_svr_ins_t o) {
    return (((uint32_t)o->m_svr_type->m_svr_type_id) << 16) | o->m_svr_id;
}

int set_svr_svr_ins_eq(set_svr_svr_ins_t l, set_svr_svr_ins_t r) {
    return l->m_svr_type->m_svr_type_id == r->m_svr_type->m_svr_type_id && l->m_svr_id == r->m_svr_id;
}
