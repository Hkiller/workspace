#include "cpe/dr/dr_metalib_manage.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "uhub_svr_ops.h"

uhub_svr_notify_info_t
uhub_svr_notify_info_create(uhub_svr_t svr, uhub_svr_info_t svr_info, uint32_t cmd) {
    uhub_svr_notify_info_t notify_info;
    int start_pos;
    LPDRMETAENTRY cmd_data_entry;
    LPDRMETA cmd_data_meta;

    cmd_data_entry = dr_meta_find_entry_by_id(svr_info->m_data_meta, cmd);
    if (cmd_data_entry == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: notify_info_create: data meta %s have no entry of cmd %d!",
            uhub_svr_name(svr), dr_meta_name(svr_info->m_data_meta), cmd);
        return NULL;
    }

    cmd_data_meta = dr_entry_ref_meta(cmd_data_entry);
    if (cmd_data_meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: notify_info_create: cmd %d entry %s no meta!",
            uhub_svr_name(svr), cmd, dr_entry_name(cmd_data_entry));
        return NULL;
    }
 
    notify_info = mem_alloc(svr->m_alloc, sizeof(struct uhub_svr_notify_info));
    if (notify_info == NULL) {
        CPE_ERROR(svr->m_em, "%s: notify_info_create: alloc fail!", uhub_svr_name(svr));
        return NULL;
    }

    notify_info->m_svr_type = svr_info->m_svr_type;
    notify_info->m_cmd = cmd;

    notify_info->m_to_uid_entry =
        dr_meta_find_entry_by_path_ex(cmd_data_meta, svr_info->m_to_uid_entry_name, &start_pos);
    if (notify_info->m_to_uid_entry == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: notify_info_create: cmd %d meta %s find entry %s fail!",
            uhub_svr_name(svr), cmd, dr_meta_name(cmd_data_meta), svr_info->m_to_uid_entry_name);
        mem_free(svr->m_alloc, notify_info);
        return NULL;
    }
    notify_info->m_to_uid_start_pos = svr_info->m_data_meta_start_pos + start_pos;

    cpe_hash_entry_init(&notify_info->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_notify_infos, notify_info) == 0) {
        CPE_ERROR(
            svr->m_em, "%s: notify_info_create: svr-type=%d, cmd=%d duplicate!",
            uhub_svr_name(svr), svr_info->m_svr_type, cmd);
        mem_free(svr->m_alloc, notify_info);
        return NULL;
    }

    return notify_info;
}

void uhub_svr_notify_info_free(uhub_svr_t svr, uhub_svr_notify_info_t notify_info) {
    cpe_hash_table_remove_by_ins(&svr->m_notify_infos, notify_info);
    mem_free(svr->m_alloc, notify_info);
}

void uhub_svr_notify_info_free_all(uhub_svr_t svr) {
    struct cpe_hash_it notify_info_it;
    uhub_svr_notify_info_t notify_info;

    cpe_hash_it_init(&notify_info_it, &svr->m_notify_infos);

    notify_info = cpe_hash_it_next(&notify_info_it);
    while(notify_info) {
        uhub_svr_notify_info_t next = cpe_hash_it_next(&notify_info_it);
        uhub_svr_notify_info_free(svr, notify_info);
        notify_info = next;
    }
}

uhub_svr_notify_info_t
uhub_svr_notify_info_find(uhub_svr_t svr, uint16_t svr_type, uint32_t cmd) {
    struct uhub_svr_notify_info key;

    key.m_svr_type = svr_type;
    key.m_cmd = cmd;

    return (uhub_svr_notify_info_t)cpe_hash_table_find(&svr->m_notify_infos, &key);    
}

uint32_t uhub_svr_notify_info_hash(uhub_svr_notify_info_t notify_info) {
    return ((uint32_t)notify_info->m_svr_type) << 16 | notify_info->m_cmd;
}

int uhub_svr_notify_info_eq(uhub_svr_notify_info_t l, uhub_svr_notify_info_t r) {
    return l->m_svr_type == r->m_svr_type && l->m_cmd == r->m_cmd;
}
