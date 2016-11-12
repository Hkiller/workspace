#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "svr/set/share/set_pkg.h"
#include "set_svr_set_conn.h"
#include "set_svr_svr_ins.h"

set_svr_set_t set_svr_set_create(set_svr_t svr, uint16_t id, uint16_t region, const char * ip, uint16_t port) {
    set_svr_set_t set;

    set = mem_alloc(svr->m_alloc, sizeof(struct set_svr_set));
    if (set == NULL) {
        CPE_ERROR(svr->m_em, "%s: create set %s.%d: malloc fail!", set_svr_name(svr), ip, port);
        return NULL;
    }

    set->m_svr = svr;
    set->m_set_id = id;
    set->m_region = region;
    cpe_str_dup(set->m_ip, sizeof(set->m_ip), ip);
    set->m_port = port;

    set->m_conn = NULL;

    TAILQ_INIT(&set->m_svr_inses);

    cpe_hash_entry_init(&set->m_hh_by_id);
    if (cpe_hash_table_insert_unique(&svr->m_sets_by_id, set) != 0) {
        CPE_ERROR(svr->m_em, "%s: set %s: id duplicate!", set_svr_name(svr), set_svr_set_name(set));
        mem_free(svr->m_alloc, set);
        return NULL;
    }

    return set;
}

void set_svr_set_free(set_svr_set_t set) {
    set_svr_t svr = set->m_svr;

    if (set->m_conn) {
        assert(set->m_conn->m_set == set);
        set_svr_set_conn_free(set->m_conn);
        assert(set->m_conn == NULL);
    }

    while(!TAILQ_EMPTY(&set->m_svr_inses)) {
        set_svr_svr_ins_free(TAILQ_FIRST(&set->m_svr_inses));
    }
    
    cpe_hash_table_remove_by_ins(&svr->m_sets_by_id, set);

    mem_free(svr->m_alloc, set);
}

void set_svr_set_free_all(set_svr_t svr) {
    struct cpe_hash_it set_it;
    set_svr_set_t set;

    cpe_hash_it_init(&set_it, &svr->m_sets_by_id);

    set = cpe_hash_it_next(&set_it);
    while(set) {
        set_svr_set_t next = cpe_hash_it_next(&set_it);
        set_svr_set_free(set);
        set = next;
    }
}

void set_svr_set_set_region(set_svr_set_t set, uint16_t region) {
    set->m_region = region;
}

void set_svr_set_set_target(set_svr_set_t set, const char * ip, uint16_t port) {
    cpe_str_dup(set->m_ip, sizeof(set->m_ip), ip);
    set->m_port = port;
    if (set->m_conn) {
        assert(set->m_conn->m_set == set);
        set_svr_set_conn_free(set->m_conn);
        assert(set->m_conn == NULL);
    }
}

set_svr_set_t set_svr_set_find_by_id(set_svr_t svr, uint32_t id) {
    struct set_svr_set key;
    key.m_set_id = id;
    return cpe_hash_table_find(&svr->m_sets_by_id, &key);
}

uint32_t set_svr_set_hash_by_id(set_svr_set_t o) {
    return o->m_set_id;
}

int set_svr_set_eq_by_id(set_svr_set_t l, set_svr_set_t r) {
    return l->m_set_id == r->m_set_id;
}

const char * set_svr_set_name(set_svr_set_t set) {
    if (set) {
        static char s_buf[32];
        snprintf(s_buf, sizeof(s_buf), "%d[%s:%d]", set->m_set_id, set->m_ip, set->m_port);
        return s_buf;
    }
    else {
        return "???";
    }
}

LPDRMETA set_svr_get_pkg_meta(set_svr_t svr, dp_req_t head, set_svr_svr_type_t to_svr_type, set_svr_svr_type_t from_svr_type) {
    switch(set_pkg_category(head)) {
    case set_pkg_request:
        if (to_svr_type == NULL) {
            to_svr_type = set_svr_svr_type_find_by_id(svr, set_pkg_to_svr_type(head));
            if (to_svr_type == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: get_pkg_meta: to svr type %d not exist!",
                    set_svr_name(svr), set_pkg_to_svr_type(head));
                return NULL;
            }
        }
        return to_svr_type->m_pkg_meta;
    case set_pkg_response:
    case set_pkg_notify: {
        if (from_svr_type == NULL) {
            from_svr_type = set_svr_svr_type_find_by_id(svr, set_pkg_from_svr_type(head));
            if (from_svr_type == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: get_pkg_meta: from svr type %d not exist!",
                    set_svr_name(svr), set_pkg_from_svr_type(head));
                return NULL;
            }
        }
        return from_svr_type->m_pkg_meta;
    }
    default:
        CPE_ERROR(
            svr->m_em, "%s: get_pkg_meta: category %d is unknown!",
            set_svr_name(svr), set_pkg_category(head));
        return NULL;
    }
}
