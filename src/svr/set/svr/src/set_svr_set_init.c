#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "set_svr_set.h"

int set_svr_app_init_set(set_svr_t set_svr, cfg_t cfg) {
    const char * str_read_block_size;
    const char * str_max_pkg_size;
    
    set_svr->m_set_timeout_ms = cfg_get_uint32(cfg, "timeout-ms", set_svr->m_set_timeout_ms);

    if ((str_read_block_size = cfg_get_string(cfg, "read-block-size", NULL))) {
        uint64_t read_block_size;
        if (cpe_str_parse_byte_size(&read_block_size, str_read_block_size) != 0) {
            CPE_ERROR(
                set_svr->m_em, "%s: init set: read read-block-size %s fail!",
                set_svr_name(set_svr), str_read_block_size);
            return -1;
        }

        set_svr->m_set_read_block_size = (uint32_t)read_block_size;
    }

    if ((str_max_pkg_size = cfg_get_string(cfg, "max-pkg-size", NULL))) {
        uint64_t max_pkg_size;
        if (cpe_str_parse_byte_size(&max_pkg_size, str_max_pkg_size) != 0) {
            CPE_ERROR(
                set_svr->m_em, "%s: init set: read max-pkg-size %s fail!",
                set_svr_name(set_svr), str_max_pkg_size);
            return -1;
        }

        set_svr->m_set_max_pkg_size = (uint32_t)max_pkg_size;
    }

    return 0;
}


