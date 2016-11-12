#include "cpe/dr/dr_metalib_manage.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "uhub_svr_ops.h"

uhub_svr_info_t uhub_svr_info_create(uhub_svr_t svr, const char * svr_type_name, const char * to_uid_entry) {
    uhub_svr_info_t svr_info;
    set_svr_svr_info_t set_svr_info;
    size_t to_uid_entry_len = strlen(to_uid_entry) + 1;
    LPDRMETAENTRY data_entry;

    set_svr_info = set_svr_svr_info_find_by_name(svr->m_stub, svr_type_name);
    if (set_svr_info == NULL) {
        CPE_ERROR(svr->m_em, "%s: svr_info_create: svr type %s unknown", uhub_svr_name(svr), svr_type_name);
        return NULL;
    }

    svr_info = mem_alloc(svr->m_alloc, sizeof(struct uhub_svr_info) + to_uid_entry_len);
    if (svr_info == NULL) {
        CPE_ERROR(svr->m_em, "%s: svr_info_create: alloc fail", uhub_svr_name(svr));
        return NULL;
    }

    svr_info->m_svr_type = set_svr_svr_info_svr_type_id(set_svr_info);
    svr_info->m_svr_info = set_svr_info;

    svr_info->m_cmd_entry = set_svr_svr_info_pkg_cmd_entry(set_svr_info);
    if (svr_info->m_cmd_entry == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: svr_info_create: svr %s no cmd entry!",
            uhub_svr_name(svr), svr_type_name);
        mem_free(svr->m_alloc, svr_info);
        return NULL;
    }


    data_entry = set_svr_svr_info_pkg_data_entry(set_svr_info);
    if (data_entry == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: svr_info_create: svr %s no data entry!",
            uhub_svr_name(svr), svr_type_name);
        mem_free(svr->m_alloc, svr_info);
        return NULL;
    }
    svr_info->m_data_meta_start_pos = (uint32_t)dr_entry_data_start_pos(data_entry, 0);

    svr_info->m_data_meta = dr_entry_ref_meta(data_entry);
    if (svr_info->m_data_meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: svr_info_create: svr %s data entry %s no meta!",
            uhub_svr_name(svr), svr_type_name, dr_entry_name(data_entry));
        mem_free(svr->m_alloc, svr_info);
        return NULL;
    }

    memcpy(svr_info + 1, to_uid_entry, to_uid_entry_len);
    svr_info->m_to_uid_entry_name = (const char *)(svr_info + 1);

    TAILQ_INSERT_TAIL(&svr->m_svr_infos, svr_info, m_next);

    return svr_info;
}

void uhub_svr_info_free(uhub_svr_t svr, uhub_svr_info_t svr_info) {
    TAILQ_REMOVE(&svr->m_svr_infos, svr_info, m_next);
    mem_free(svr->m_alloc, svr_info);
}

uhub_svr_info_t uhub_svr_info_find(uhub_svr_t svr, uint16_t svr_type) {
    uhub_svr_info_t svr_info;

    TAILQ_FOREACH(svr_info, &svr->m_svr_infos, m_next) {
        if (svr_info->m_svr_type == svr_type) return svr_info;
    }

    return NULL;
}


