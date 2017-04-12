#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/logic/logic_data.h"
#include "usf/logic_use/logic_data_buf.h"
#include "protocol/logic_use/logic_data_buf_info.h"

extern char g_metalib_logic_use[];

LPDRMETA logic_data_buf_meta(void) {
    return dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_logic_use, "logic_data_buf_info");
}

logic_data_t logic_data_buf_find(logic_require_t r) {
    return logic_require_data_find(r, "logic_data_buf_info");
}

logic_data_t logic_data_buf_get_or_create(logic_require_t r, size_t capacity) {
    LPDRMETA meta;

    meta  = logic_data_buf_meta();
    assert(meta);

    return meta
        ? logic_require_data_get_or_create(r, meta, sizeof(LOGIC_DATA_BUF_INFO) + capacity)
        : NULL;
}

int logic_data_buf_capacity(logic_data_t data) {
    return logic_data_capacity(data) - sizeof(LOGIC_DATA_BUF_INFO);
}

int logic_data_buf_size(logic_data_t data) {
    LOGIC_DATA_BUF_INFO * d = logic_data_data(data);
    return d->size;
}

void * logic_data_buf(logic_data_t data) {
    LOGIC_DATA_BUF_INFO * d = logic_data_data(data);
    return d + 1;
}
