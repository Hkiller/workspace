#include "cpe/dr/dr_metalib_manage.h"
#include "svr/set/stub/set_svr_stub.h"
#include "set_svr_stub_internal_ops.h"

set_svr_cmd_info_t
set_svr_cmd_info_create(set_svr_stub_t stub, set_svr_svr_info_t svr_info, LPDRMETAENTRY entry) {
    set_svr_cmd_info_t svr_cmd;
    const char * meta_name;

    svr_cmd = mem_alloc(stub->m_alloc, sizeof(struct set_svr_cmd_info));
    if (svr_cmd == NULL) return NULL;

    meta_name = dr_meta_name(dr_entry_ref_meta(entry));

    svr_cmd->m_meta_name = meta_name;
    svr_cmd->m_entry = entry;

    cpe_hash_entry_init(&svr_cmd->m_hh);
    if (cpe_hash_table_insert_unique(&svr_info->m_cmds, svr_cmd) != 0) {
        CPE_ERROR(
            stub->m_em, "%s: create cmd info %s.%s: duplicate!",
            set_svr_stub_name(stub), svr_info->m_svr_type_name, meta_name);
        mem_free(stub->m_alloc, svr_cmd);
        return NULL;
    }
    
    return svr_cmd;
}

void set_svr_cmd_info_free(set_svr_stub_t stub, set_svr_svr_info_t svr_info, set_svr_cmd_info_t svr_cmd) {
    cpe_hash_table_remove_by_ins(&svr_info->m_cmds, svr_cmd);
    mem_free(stub->m_alloc, svr_cmd);
}

set_svr_cmd_info_t
set_svr_cmd_info_find_by_name(set_svr_stub_t stub, set_svr_svr_info_t svr_info, const char * meta_name) {
    struct set_svr_cmd_info key;
    key.m_meta_name = meta_name;
    return cpe_hash_table_find(&svr_info->m_cmds, &key);
}

void set_svr_cmd_info_free_all(set_svr_stub_t stub, set_svr_svr_info_t svr_info) {
    struct cpe_hash_it svr_cmd_it;
    set_svr_cmd_info_t svr_cmd;

    cpe_hash_it_init(&svr_cmd_it, &svr_info->m_cmds);

    svr_cmd = cpe_hash_it_next(&svr_cmd_it);
    while (svr_cmd) {
        set_svr_cmd_info_t next = cpe_hash_it_next(&svr_cmd_it);
        set_svr_cmd_info_free(stub, svr_info, svr_cmd);
        svr_cmd = next;
    }
}

uint32_t set_svr_cmd_info_hash(set_svr_cmd_info_t cmd_info) {
    return cpe_hash_str(cmd_info->m_meta_name, strlen(cmd_info->m_meta_name));
}

int set_svr_cmd_info_eq(set_svr_cmd_info_t l, set_svr_cmd_info_t r) {
    return strcmp(l->m_meta_name, r->m_meta_name) == 0;
}

