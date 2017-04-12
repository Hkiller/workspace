#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "uhub_svr_ops.h"

int uhub_svr_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    uhub_svr_t svr = ctx;
    dp_req_t pkg_head;
    uhub_svr_info_t from_svr_info;
    uint32_t cmd;
    uhub_svr_notify_info_t notify_info;
    LPDRMETA data_meta;
    //LPDRMETAENTRY to_uid_entry;

    pkg_head = set_pkg_head_find(req);
    if (pkg_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: process: find pkg head fail!", uhub_svr_name(svr));
        return -1;
    }

    if (set_pkg_category(pkg_head) != set_pkg_notify) {
        CPE_ERROR(svr->m_em, "%s: process: receive pkg is not notify!", uhub_svr_name(svr));
        return -1;
    }

    from_svr_info = uhub_svr_info_find(svr, set_pkg_from_svr_type(pkg_head));
    if (from_svr_info == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: process: receive notify from svr %d, svr type not exist!",
            uhub_svr_name(svr), set_pkg_from_svr_type(pkg_head));
        return -1;
    }

    if (dr_entry_try_read_uint32(
            &cmd,
            ((char*)dp_req_data(req)) + dr_entry_data_start_pos(from_svr_info->m_cmd_entry, 0),
            from_svr_info->m_cmd_entry, svr->m_em)
        != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: process: receive notify from svr %d, svr %s read cmd from %s fail!",
            uhub_svr_name(svr), set_pkg_from_svr_type(pkg_head),
            set_svr_svr_info_svr_type_name(from_svr_info->m_svr_info), dr_entry_name(from_svr_info->m_cmd_entry));
        return -1;
    }

    data_meta = set_svr_svr_info_find_data_meta_by_cmd(from_svr_info->m_svr_info, cmd);
    if (data_meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: process: receive notify from svr %d, svr %s cmd %d no data!",
            uhub_svr_name(svr), set_pkg_from_svr_type(pkg_head),
            set_svr_svr_info_svr_type_name(from_svr_info->m_svr_info), cmd);
        return -1;
    }

    notify_info = uhub_svr_notify_info_find(svr, from_svr_info->m_svr_type, cmd);
    if (notify_info == NULL) {
        notify_info = uhub_svr_notify_info_create(svr, from_svr_info, cmd);
        if (notify_info == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: process: create notify info of svr %d cmd %d fail!",
                uhub_svr_name(svr), set_pkg_from_svr_type(pkg_head), cmd);
            return -1;
        }
    }

    //to_uid_entry = dr_meta_find_entry_by_path_ex
    /* pkg = dp_req_data(req); */

    /* if (pkg->cmd >= (sizeof(g_svr_ops) / sizeof(g_svr_ops[0])) */
    /*     || g_svr_ops[pkg->cmd] == NULL) */
    /* { */
    /*     CPE_ERROR(svr->m_em, "%s: process: not support cmd %d!", uhub_svr_name(svr), pkg->cmd); */
    /*     return -1; */
    /* } */
    
    /* g_svr_ops[pkg->cmd](svr, pkg_head, req); */

    return 0;
}
