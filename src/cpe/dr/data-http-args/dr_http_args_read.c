#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/http_args.h"
#include "cpe/dr/dr_http_args.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

int dr_http_args_read_args(
    void * result,
    size_t capacity,
    cpe_http_arg_t args,
    uint16_t arg_count,
    LPDRMETA meta,
    error_monitor_t em)
{
    uint16_t i;

    for(i = 0; i < arg_count; ++i) {
        cpe_http_arg_t one = args + i;
        LPDRMETAENTRY entry;
        
        entry = dr_meta_find_entry_by_name(meta, one->name);
        if (entry == NULL) continue;

        if (dr_entry_size(entry) + dr_entry_data_start_pos(entry, 0) > capacity) continue;
        
        if (dr_entry_set_from_string(((char *)result) + dr_entry_data_start_pos(entry, 0), one->value, entry, em) != 0) {
            CPE_ERROR(
                em, "dr_http_args_read_args: entry %s type %s set from str %s fail!",
                dr_entry_name(entry), dr_entry_type_name(entry), one->value);
            return -1;
        }
    }

    return 0;
}


