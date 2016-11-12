#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "center_svr_type.h"

static int center_svr_load_svr_svr_types(center_svr_t svr, cfg_t svr_types) {
    struct cfg_it svr_it;
    cfg_t svr_cfg;

    cfg_it_init(&svr_it, svr_types);
    while((svr_cfg = cfg_it_next(&svr_it))) {
        center_svr_type_t svr_type;
        const char * svr_type_name = cfg_name(svr_cfg);
        uint16_t svr_type_id = cfg_get_uint16(svr_cfg, "id", 0);

        if (center_svr_type_lsearch_by_name(svr, svr_type_name) != NULL) {
            CPE_INFO(svr->m_em, "%s: load svr config: svr %s duplicate", center_svr_name(svr), svr_type_name);
            return -1;
        }

        if (svr_type_id == 0) {
            CPE_ERROR(svr->m_em, "%s: load svr config: svr %s id invalid", center_svr_name(svr), svr_type_name);
            return -1;
        }

        if (center_svr_type_find(svr, svr_type_id)) {
            CPE_ERROR(svr->m_em, "%s: load svr config: svr %s id %d duplicate!", center_svr_name(svr), svr_type_name, svr_type_id);
            return -1;
        }

        svr_type = center_svr_type_create(svr, svr_type_name, svr_type_id);
        if (svr_type == NULL) {
            CPE_ERROR(svr->m_em, "%s: load svr config: svr %s(%d) create fail!", center_svr_name(svr), svr_type_name, svr_type_id);
            return -1;
        }

        if (svr->m_debug) {
            CPE_INFO(svr->m_em, "%s: load svr config: svr %s(%d) load success!", center_svr_name(svr), svr_type_name, svr_type_id);
        }
    }

    return 0;
}

int center_svr_load_svr_config(center_svr_t svr) {
    cfg_t svr_types;

    svr_types = cfg_find_cfg(gd_app_cfg(svr->m_app), "svr_types");
    if (svr_types == NULL) {
        CPE_ERROR(svr->m_em, "%s: load svr config: no config data", center_svr_name(svr));
        return -1;
    }

    if (center_svr_load_svr_svr_types(svr, svr_types) != 0) {
        return -1;
    }

    return 0;
}
