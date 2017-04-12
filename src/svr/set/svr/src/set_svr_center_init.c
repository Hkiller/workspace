#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "set_svr_center_fsm.h"

int set_svr_app_init_center(set_svr_center_t center, cfg_t cfg, const char * center_addr) {
    const char * str_read_block_size;
    const char * str_max_pkg_size;

    center->m_reconnect_span_ms = cfg_get_uint32(cfg, "reconnect-span-ms", center->m_reconnect_span_ms);
    center->m_update_span_s = cfg_get_uint32(cfg, "update-span-s", center->m_update_span_s);

    if ((str_read_block_size = cfg_get_string(cfg, "read-block-size", NULL))) {
        uint64_t read_block_size;
        if (cpe_str_parse_byte_size(&read_block_size, str_read_block_size) != 0) {
            CPE_ERROR(
                center->m_svr->m_em, "%s: create: center: read read-block-size %s fail!",
                set_svr_name(center->m_svr), str_read_block_size);
            return -1;
        }

        center->m_read_block_size = (uint32_t)read_block_size;
    }

    if ((str_max_pkg_size = cfg_get_string(cfg, "max-pkg-size", NULL))) {
        uint64_t max_pkg_size;
        if (cpe_str_parse_byte_size(&max_pkg_size, str_max_pkg_size) != 0) {
            CPE_ERROR(
                center->m_svr->m_em, "%s: create: center: read max-pkg-size %s fail!",
                set_svr_name(center->m_svr), str_max_pkg_size);
            return -1;
        }

        center->m_max_pkg_size = (uint32_t)max_pkg_size;
    }

    if (center_addr) {
        char ip[64];
        short port;
        const char * sep;

        sep = strchr(center_addr, ':');
        if (sep) {
            int len = sep - center_addr;

            if ((len + 1)  >= sizeof(ip)) {
                CPE_ERROR(center->m_svr->m_em, "%s: create: center: center_addr %s too long!", set_svr_name(center->m_svr), center_addr);
                return -1;
            }

            memcpy(ip, center_addr, len);
            ip[len] = 0;
            port = atoi(sep + 1);
        }
        else {
            int len = strlen(center_addr);

            memcpy(ip, center_addr, len);
            ip[len] = 0;
            port = 8099;
        }

        if (set_svr_center_set_svr(center, ip, port) != 0) {
            CPE_ERROR(
                center->m_svr->m_em, "%s: create: center: set svr %s:%d fail!",
                set_svr_name(center->m_svr), ip, (int)port);
            return -1;
        }

        set_svr_center_apply_evt(center, set_svr_center_fsm_evt_start);
    }
    else {
        if (center->m_svr->m_debug) {
            CPE_INFO(center->m_svr->m_em, "%s: create: center: no center ip!", set_svr_name(center->m_svr));
        }
    }

    return 0;
}

