#include <assert.h>
#include "protocol/base/base_package.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_request.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "bpg_pkg_internal_types.h"

int bpg_pkg_build_from_cfg(dp_req_t body, cfg_t cfg, error_monitor_t em) {
    LPDRMETALIB metalib;
    LPDRMETA meta;
    LPDRMETA main_data_meta;
    struct cfg_it cfg_it;
    cfg_t child_cfg;
    bpg_pkg_t pkg = bpg_pkg_find(body);

    char * output_buf = (char *)dp_req_data(body);
    size_t output_buf_capacity = dp_req_capacity(body);
    BASEPKG * output_pkg = (BASEPKG *)output_buf;

    assert(pkg);
    assert(pkg->m_mgr);

    metalib = bpg_pkg_manage_data_metalib(pkg->m_mgr);
    if (metalib == NULL) {
        CPE_ERROR(
            em, "%s: build from cfg:  data meta not exist!",
            bpg_pkg_manage_name(pkg->m_mgr));
        return -1;
    }

    meta = bpg_pkg_manage_basepkg_head_meta(pkg->m_mgr);
    if (meta == NULL) {
        CPE_ERROR(
            em, "%s: build from cfg: head_meta not exist!",
            bpg_pkg_manage_name(pkg->m_mgr));
        return -1;
    }

    if (dr_cfg_read(output_buf, sizeof(BASEPKG_HEAD), cfg, meta, 0, NULL) < 0) {
        CPE_ERROR(
            em, "%s: build pkg from cfg: read head fail!",
            bpg_pkg_manage_name(pkg->m_mgr));
        return -1;
    }

    output_buf_capacity -= sizeof(BASEPKG_HEAD);
    output_buf += sizeof(BASEPKG_HEAD);

    output_pkg->head.bodylen = 0;
    output_pkg->head.bodytotallen = 0;
    output_pkg->head.appendInfoCount = 0;

    main_data_meta = bpg_pkg_main_data_meta(body, em);
    if (main_data_meta) {
        int use_size;

        cfg_it_init(&cfg_it, cfg);
        while((child_cfg = cfg_it_next(&cfg_it))) {
            if (cfg_type(child_cfg) != CPE_CFG_TYPE_STRUCT) continue;
            if (strcmp(cfg_name(child_cfg), dr_meta_name(main_data_meta)) != 0) continue;

            use_size = dr_cfg_read(output_buf, output_buf_capacity, child_cfg, main_data_meta, 0, NULL);
            if (use_size < 0) {
                CPE_ERROR(
                    em, "%s: build pkg from cfg: read main body %s fail!",
                    bpg_pkg_manage_name(pkg->m_mgr), dr_meta_name(main_data_meta));
                return -1;
            }

            output_pkg->head.bodylen = use_size;
            output_pkg->head.bodytotallen = use_size;
            output_buf += use_size;
        }
    }

    cfg_it_init(&cfg_it, cfg);
    while((child_cfg = cfg_it_next(&cfg_it))) {
        APPENDINFO * o_append_info;
        int use_size;

        if (cfg_type(child_cfg) != CPE_CFG_TYPE_STRUCT) continue;
        if (main_data_meta
            && strcmp(cfg_name(child_cfg), dr_meta_name(main_data_meta)) == 0) continue;

        meta = dr_lib_find_meta_by_name(metalib, cfg_name(child_cfg));
        if (meta == NULL) continue;

        use_size = dr_cfg_read(output_buf, output_buf_capacity, child_cfg, meta, 0, NULL);
        if (use_size < 0) {
            CPE_ERROR(
                em, "%s: build pkg from cfg: read append body %s fail!",
                bpg_pkg_manage_name(pkg->m_mgr), dr_meta_name(meta));
            return -1;
        }

        if (output_pkg->head.appendInfoCount
            >= (sizeof(output_pkg->head.appendInfos) / sizeof(output_pkg->head.appendInfos[0])))
        {
            CPE_ERROR(
                em, "%s: build pkg from cfg: too many append info!",
                bpg_pkg_manage_name(pkg->m_mgr));
            return -1;
        }

        o_append_info = &output_pkg->head.appendInfos[output_pkg->head.appendInfoCount++];
        o_append_info->id = dr_meta_id(meta);
        o_append_info->size = use_size;
        output_pkg->head.bodytotallen += use_size;
    }

    dp_req_set_size(body, output_pkg->head.bodytotallen);

    return 0;
}
