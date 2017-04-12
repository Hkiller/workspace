#include "cpe/dr/dr_metalib_manage.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "svr/conn/net_cli/conn_net_cli_svr_stub.h"
#include "conn_net_cli_internal_ops.h"

conn_net_cli_cmd_info_t
conn_net_cli_cmd_info_create(conn_net_cli_svr_stub_t stub, LPDRMETAENTRY entry) {
    conn_net_cli_cmd_info_t svr_cmd;
    const char * meta_name;

    svr_cmd = mem_alloc(stub->m_cli->m_alloc, sizeof(struct conn_net_cli_cmd_info));
    if (svr_cmd == NULL) return NULL;

    meta_name = dr_meta_name(dr_entry_ref_meta(entry));

    svr_cmd->m_meta_name = meta_name;
    svr_cmd->m_entry = entry;

    cpe_hash_entry_init(&svr_cmd->m_hh);
    if (cpe_hash_table_insert_unique(&stub->m_cmds, svr_cmd) != 0) {
        CPE_ERROR(
            stub->m_cli->m_em, "%s: create cmd info %s.%s: duplicate!",
            conn_net_cli_name(stub->m_cli), stub->m_svr_type_name, meta_name);
        mem_free(stub->m_cli->m_alloc, svr_cmd);
        return NULL;
    }
    
    return svr_cmd;
}

void conn_net_cli_cmd_info_free(conn_net_cli_svr_stub_t stub, conn_net_cli_cmd_info_t svr_cmd) {
    cpe_hash_table_remove_by_ins(&stub->m_cmds, svr_cmd);
    mem_free(stub->m_cli->m_alloc, svr_cmd);
}

conn_net_cli_cmd_info_t
conn_net_cli_cmd_info_find_by_name(conn_net_cli_svr_stub_t stub, const char * meta_name) {
    struct conn_net_cli_cmd_info key;
    key.m_meta_name = meta_name;
    return cpe_hash_table_find(&stub->m_cmds, &key);
}

void conn_net_cli_cmd_info_free_all(conn_net_cli_svr_stub_t stub) {
    struct cpe_hash_it svr_cmd_it;
    conn_net_cli_cmd_info_t svr_cmd;

    cpe_hash_it_init(&svr_cmd_it, &stub->m_cmds);

    svr_cmd = cpe_hash_it_next(&svr_cmd_it);
    while (svr_cmd) {
        conn_net_cli_cmd_info_t next = cpe_hash_it_next(&svr_cmd_it);
        conn_net_cli_cmd_info_free(stub, svr_cmd);
        svr_cmd = next;
    }
}

uint32_t conn_net_cli_cmd_info_hash(conn_net_cli_cmd_info_t cmd_info) {
    return cpe_hash_str(cmd_info->m_meta_name, strlen(cmd_info->m_meta_name));
}

int conn_net_cli_cmd_info_eq(conn_net_cli_cmd_info_t l, conn_net_cli_cmd_info_t r) {
    return strcmp(l->m_meta_name, r->m_meta_name) == 0;
}

