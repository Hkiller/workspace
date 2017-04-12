#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/range_bitarry.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "aom_internal_types.h"
#include "aom_data.h"

void aom_obj_mgr_info(aom_obj_mgr_t mgr, write_stream_t stream, int ident) {
    struct aom_obj_control_data * control;

    stream_putc_count(stream, ' ', ident << 2);
    stream_printf(stream, "aom info:\n");

    control = (struct aom_obj_control_data *)mgr->m_full_base;

    stream_putc_count(stream, ' ', ident << 2);
    stream_printf(stream, "buf info: version=%d, total-size=%d\n", control->m_head_version, mgr->m_full_capacity);

    stream_putc_count(stream, ' ', (ident + 1) << 2);
    stream_printf(stream, "metalib-buf: start=%d, size=%d\n", control->m_metalib_start, control->m_metalib_size);

    stream_putc_count(stream, ' ', (ident + 1) << 2);
    stream_printf(stream, "data-buf: start=%d, size=%d\n", control->m_data_start, control->m_data_size);
}

