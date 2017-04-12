#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "set_svr_center_op.h"
#include "set_svr_svr_ins.h"
#include "set_svr_svr_ins.h"
#include "set_svr_set.h"

static int set_svr_svr_info_cmp(void const * i_l, void const * i_r);
static void set_svr_center_ins_check_remove(set_svr_t svr, set_svr_svr_ins_t svr_ins, uint16_t svr_info_count, SVR_CENTER_SVR_INFO const * svr_infos);
static void set_svr_center_ins_check_create(set_svr_t svr, SVR_CENTER_SVR_INFO const * svr_info);

void set_svr_center_send_sync_cmd(set_svr_center_t center) {
    set_svr_t svr = center->m_svr;
    SVR_CENTER_PKG * pkg;
    size_t pkg_capacity;

    pkg_capacity = sizeof(SVR_CENTER_PKG);
    pkg = set_svr_center_get_pkg_buff(center, pkg_capacity);
    if (pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send syncing: get pkg buf fail!", set_svr_name(svr));
        return;
    }

    pkg->cmd = SVR_CENTER_CMD_REQ_QUERY;
    
    if (set_svr_center_send(center, pkg, pkg_capacity) != 0) {
        CPE_ERROR(svr->m_em, "%s: send syncing req fail!", set_svr_name(svr));
    }
    else {
        if (svr->m_debug) {
            CPE_INFO(svr->m_em, "%s: send syncing!", set_svr_name(svr));
        }
    }
}

void set_svr_center_on_sync_svrs(set_svr_center_t center, SVR_CENTER_RES_QUERY * syncing_res) {
    set_svr_t svr = center->m_svr;
    uint16_t i;
    struct cpe_hash_it svr_ins_it;
    set_svr_svr_ins_t svr_ins, next_svr_ins;

    /*清理掉不存在的绑定 */
    qsort(syncing_res->data, syncing_res->count, sizeof(syncing_res->data[0]), set_svr_svr_info_cmp);

    cpe_hash_it_init(&svr_ins_it, &svr->m_svr_inses);
    for(svr_ins = cpe_hash_it_next(&svr_ins_it); svr_ins; svr_ins = next_svr_ins) {
        next_svr_ins = cpe_hash_it_next(&svr_ins_it);
        set_svr_center_ins_check_remove(svr, svr_ins, syncing_res->count, syncing_res->data);
    }
    
    for(i = 0; i < syncing_res->count; ++i) {
        set_svr_center_ins_check_create(svr, &syncing_res->data[i]);
    }
}

void set_svr_center_on_ntf_join(set_svr_center_t center, SVR_CENTER_NTF_JOIN * ntf_join) {
    set_svr_t svr = center->m_svr;
    uint16_t i;
    set_svr_set_t set;

    set = set_svr_set_find_by_id(svr, ntf_join->set_id);
    if (set) {
        set_svr_svr_ins_t ins, next_ins;

        qsort(ntf_join->data, ntf_join->count, sizeof(ntf_join->data[0]), set_svr_svr_info_cmp);

        for(ins = TAILQ_FIRST(&set->m_svr_inses); ins; ins = next_ins) {
            next_ins = TAILQ_NEXT(ins, m_next_for_set);

            set_svr_center_ins_check_remove(svr, ins, ntf_join->count, ntf_join->data);
        }
    }

    for(i = 0; i < ntf_join->count; ++i) {
        set_svr_center_ins_check_create(svr, &ntf_join->data[i]);
    }
}

void set_svr_center_on_ntf_leave(set_svr_center_t center, SVR_CENTER_NTF_LEAVE * ntf_leave) {
    set_svr_t svr = center->m_svr;
    set_svr_set_t set;
    
    set = set_svr_set_find_by_id(svr, ntf_leave->set_id);
    if (set == NULL) return;

    while(!TAILQ_EMPTY(&set->m_svr_inses)) {
        set_svr_svr_ins_t ins = TAILQ_FIRST(&set->m_svr_inses);
        
        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: ntf leave: %d.%d remove for set %d leave",
                set_svr_name(svr), ins->m_svr_type->m_svr_type_id, ins->m_svr_id, ntf_leave->set_id);
        }
        
        set_svr_svr_ins_free(ins);
    }
}

static void set_svr_center_ins_check_create(set_svr_t svr, SVR_CENTER_SVR_INFO const * svr_info) {
    set_svr_set_t set;
    set_svr_svr_ins_t svr_ins;
    set_svr_svr_type_t svr_type;

    svr_ins = set_svr_svr_ins_find(svr, svr_info->svr.svr_type, svr_info->svr.svr_id);
    if (svr_ins && svr_ins->m_set == NULL) {
        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: sync svrs: %d.%d is local",
                set_svr_name(svr), svr_info->svr.svr_type, svr_info->svr.svr_id);
        }
        return;
    }
            
    set = set_svr_set_find_by_id(svr, svr_info->set.id);
    if (set == NULL) {
        set = set_svr_set_create(svr, svr_info->set.id, svr_info->set.region, svr_info->set.ip, svr_info->set.port);
        if (set == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: sync svrs: create set %s.%d fail",
                set_svr_name(svr), svr_info->set.ip, svr_info->set.port);
            return;
        }

        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: sync svrs: create set %s",
                set_svr_name(svr), set_svr_set_name(set));
        }
    }
    else {
        if (set->m_region != svr_info->set.region) {
            CPE_INFO(
                svr->m_em, "%s: sync svrs: set %s region %d ==> %d",
                set_svr_name(svr), set_svr_set_name(set),
                set->m_region, svr_info->set.region);

            set_svr_set_set_region(set, svr_info->set.region);
        }
            
        if (strcmp(set->m_ip, svr_info->set.ip) != 0 || set->m_port != svr_info->set.port) {
            CPE_INFO(
                svr->m_em, "%s: sync svrs: set %s target ==>  %d-%s:%d",
                set_svr_name(svr), set_svr_set_name(set),
                svr_info->set.id, svr_info->set.ip, svr_info->set.port);

            set_svr_set_set_target(set, svr_info->set.ip, svr_info->set.port);
        }
    }
        
    if (svr_ins) {
        assert(svr_ins->m_set == set);
        return;
    }

    assert(svr_ins == NULL);

    svr_type = set_svr_svr_type_find_by_id(svr, svr_info->svr.svr_type);
    if (svr_type == NULL) {
        if (svr->m_debug) {
            CPE_INFO(svr->m_em, "%s: sync svrs: svr type %d not exist", set_svr_name(svr), svr_info->svr.svr_type);
        }
        return;
    }

    svr_ins = set_svr_svr_ins_create(svr_type, svr_info->svr.svr_id, set);
    if (svr_ins == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: sync svrs: create svr ins %d-%d fail", set_svr_name(svr),
            svr_info->svr.svr_type, svr_info->svr.svr_id);
        return;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: sync svrs: create svr ins %d-%d at set %s",
            set_svr_name(svr), svr_info->svr.svr_type, svr_info->svr.svr_id,
            set_svr_set_name(set));
    }
}

static void set_svr_center_ins_check_remove(set_svr_t svr, set_svr_svr_ins_t svr_ins, uint16_t svr_info_count, SVR_CENTER_SVR_INFO const * svr_infos) {
    SVR_CENTER_SVR_INFO key;
    SVR_CENTER_SVR_INFO * svr_info;
    set_svr_set_t set;

    key.svr.svr_type = svr_ins->m_svr_type->m_svr_type_id;
    key.svr.svr_id = svr_ins->m_svr_id;

    svr_info = bsearch(&key, svr_infos, svr_info_count, sizeof(key), set_svr_svr_info_cmp);
    if (svr_info == NULL) {
        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: sync svrs: %d.%d not exist, remove",
                set_svr_name(svr), key.svr.svr_type, key.svr.svr_id);
        }
    }
    else {
        if (svr_info->set.id == svr->m_set_id) {
            if (svr_ins->m_set != NULL) {
                if (svr->m_debug) {
                    CPE_INFO(
                        svr->m_em, "%s: sync svrs: %d.%d need move to local, remove first",
                        set_svr_name(svr), key.svr.svr_type, key.svr.svr_id);
                }
            }
            else {
                return;
            }
        }
        else {
            if (svr_ins->m_set == NULL || svr_ins->m_set->m_set_id != svr_info->set.id) {
                if (svr->m_debug) {
                    CPE_INFO(
                        svr->m_em, "%s: sync svrs: %d.%d set changed, remove first",
                        set_svr_name(svr), key.svr.svr_type, key.svr.svr_id);
                }
            }
            else {
                return;
            }
        }
    }

    set = svr_ins->m_set;

    set_svr_svr_ins_free(svr_ins);

    if (set && TAILQ_EMPTY(&set->m_svr_inses)) {
        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: sync svrs: set %d already empty, remove",
                set_svr_name(svr), set->m_set_id);
        }
        set_svr_set_free(set);
        set = NULL;
    }
}

static int set_svr_svr_info_cmp(void const * i_l, void const * i_r) {
    SVR_CENTER_SVR_INFO const * l = (SVR_CENTER_SVR_INFO const *)i_l;
    SVR_CENTER_SVR_INFO const * r = (SVR_CENTER_SVR_INFO const *)i_r;

    return l->svr.svr_id != r->svr.svr_id
        ? (int)l->svr.svr_id - (int)r->svr.svr_id
        : (int)l->svr.svr_type - (int)r->svr.svr_type;
}
