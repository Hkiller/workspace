#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_validate.h"

static void dr_metalib_validate_align_i(error_monitor_t em, LPDRMETALIB metalib) {
    int meta_pos;
    int meta_count;

    meta_count = dr_lib_meta_num(metalib);
    for(meta_pos = 0; meta_pos < meta_count; ++meta_pos) {
        LPDRMETA meta;
        int entry_pos;
        int entry_count;

        meta = dr_lib_meta_at(metalib, meta_pos);
        entry_count = dr_meta_entry_num(meta);

        for(entry_pos = 0; entry_pos < entry_count; ++entry_pos) {
            LPDRMETAENTRY entry = dr_meta_entry_at(meta, entry_pos);
            size_t align = dr_entry_require_align(entry);
            if (align != 1 && align != 2 && align != 4 && align != 8) {
                CPE_ERROR(
                    em, "%s.%s: type align %d error",
                    dr_meta_name(meta), dr_entry_name(entry), (int)align);
                continue;
            }

            if (dr_entry_data_start_pos(entry, 0) % align) {
                CPE_ERROR(
                    em, "%s.%s: start pos error, align is %d, startpos is %d",
                    dr_meta_name(meta), dr_entry_name(entry),
                    (int)align, (int)dr_entry_data_start_pos(entry, 0));
                continue;
            }
        }
    }
}

int dr_metalib_validate_align(error_monitor_t em, LPDRMETALIB metalib) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        dr_metalib_validate_align_i(em, metalib);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        dr_metalib_validate_align_i(&logError, metalib);
    }

    return ret;
}
