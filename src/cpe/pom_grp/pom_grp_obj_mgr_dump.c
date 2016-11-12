#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/range_bitarry.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom/pom_class.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"
#include "pom_grp_data.h"

static void pom_grp_obj_mgr_info_class(pom_class_t pom_class, write_stream_t stream, int ident) {
    stream_putc_count(stream, ' ', ident << 2);
    stream_printf(
        stream, "class %s(%d): obj-size=%d, obj-per-page=%d\n",
        pom_class_name(pom_class),
        pom_class_id(pom_class),
        (int)pom_class_obj_size(pom_class),
        (int)pom_class_object_per_page(pom_class)
        );
}

static void pom_mgr_obj_mgr_buf_info(pom_grp_obj_mgr_t mgr, write_stream_t stream, int ident) {
    struct pom_grp_obj_control_data * control;

    control = (struct pom_grp_obj_control_data *)mgr->m_full_base;

    stream_putc_count(stream, ' ', ident << 2);
    stream_printf(stream, "buf info: version=%d, total-size=%d\n", control->m_head_version, mgr->m_full_capacity);

    stream_putc_count(stream, ' ', (ident + 1) << 2);
    stream_printf(stream, "obj-meta-buf: start=%d, size=%d\n", control->m_objmeta_start, control->m_objmeta_size);

    stream_putc_count(stream, ' ', (ident + 1) << 2);
    stream_printf(stream, "metalib-buf: start=%d, size=%d\n", control->m_metalib_start, control->m_metalib_size);

    stream_putc_count(stream, ' ', (ident + 1) << 2);
    stream_printf(stream, "data-buf: start=%d, size=%d\n", control->m_data_start, control->m_data_size);
}

void pom_grp_obj_mgr_info(pom_grp_obj_mgr_t mgr, write_stream_t stream, int ident) {
    uint32_t i;

    stream_putc_count(stream, ' ', ident << 2);
    stream_printf(stream, "pom info:\n");
    pom_grp_obj_mgr_info_class(pom_mgr_get_class(mgr->m_omm,  mgr->m_meta->m_control_class_id), stream, ident + 1);
    for(i = 0; i < mgr->m_meta->m_entry_count; ++i) {
        pom_grp_entry_meta_t entry_meta = mgr->m_meta->m_entry_buf[i];

        pom_grp_obj_mgr_info_class(pom_mgr_get_class(mgr->m_omm,  entry_meta->m_class_id), stream, ident + 1);
    }
    stream_printf(stream, "\n");

    pom_grp_meta_dump(stream, mgr->m_meta, ident);
    stream_printf(stream, "\n");
    stream_printf(stream, "\n");

    pom_mgr_obj_mgr_buf_info(mgr, stream, ident);
}

