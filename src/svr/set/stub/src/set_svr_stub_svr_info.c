#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/stub/set_svr_stub.h"
#include "set_svr_stub_internal_ops.h"

set_svr_svr_info_t set_svr_svr_info_create(set_svr_stub_t stub, const char * svr_name, uint16_t svr_type) {
    set_svr_svr_info_t svr_info;

    svr_info = mem_alloc(stub->m_alloc, sizeof(struct set_svr_svr_info));
    if (svr_info == NULL) return NULL;

    svr_info->m_svr_type_id = svr_type;
    cpe_str_dup(svr_info->m_svr_type_name, sizeof(svr_info->m_svr_type_name), svr_name);

    svr_info->m_pkg_meta = NULL;
    svr_info->m_pkg_cmd_entry = NULL;
    svr_info->m_pkg_data_entry = NULL;
    svr_info->m_carry_meta = NULL;

    svr_info->m_error_pkg_meta = NULL;
    svr_info->m_error_pkg_cmd = 0;
    svr_info->m_error_pkg_error_entry = NULL;

    svr_info->m_notify_dispatch_to = NULL;
    svr_info->m_response_dispatch_to = NULL;

    if (cpe_hash_table_init(
            &svr_info->m_cmds,
            stub->m_alloc,
            (cpe_hash_fun_t) set_svr_cmd_info_hash,
            (cpe_hash_eq_t) set_svr_cmd_info_eq,
            CPE_HASH_OBJ2ENTRY(set_svr_cmd_info, m_hh),
            -1) != 0)
    {
        CPE_ERROR(
            stub->m_em, "%s: create svr info %s(%d): init cmd hash table fail!",
            set_svr_stub_name(stub), svr_name, svr_type);
        mem_free(stub->m_alloc, svr_info);
        return NULL;
    }

    cpe_hash_entry_init(&svr_info->m_hh);
    if (cpe_hash_table_insert_unique(&stub->m_svr_infos, svr_info) != 0) {
        CPE_ERROR(
            stub->m_em, "%s: create svr info %s(%d): svr_type_id duplicate!",
            set_svr_stub_name(stub), svr_name, svr_type);
        cpe_hash_table_fini(&svr_info->m_cmds);
        mem_free(stub->m_alloc, svr_info);
        return NULL;
    }
    
    return svr_info;
}

void set_svr_svr_info_free(set_svr_stub_t stub, set_svr_svr_info_t svr_info) {
    set_svr_cmd_info_free_all(stub, svr_info);
    cpe_hash_table_fini(&svr_info->m_cmds);

    if (svr_info->m_notify_dispatch_to) {
        mem_free(stub->m_alloc, svr_info->m_notify_dispatch_to);
    }

    if (svr_info->m_response_dispatch_to) {
        mem_free(stub->m_alloc, svr_info->m_response_dispatch_to);
    }

    mem_free(stub->m_alloc, svr_info);
}

set_svr_svr_info_t set_svr_svr_info_find(set_svr_stub_t stub, uint16_t svr_type) {
    struct set_svr_svr_info key;
    key.m_svr_type_id = svr_type;
    return cpe_hash_table_find(&stub->m_svr_infos, &key);
}

set_svr_svr_info_t set_svr_svr_info_find_by_name(set_svr_stub_t stub, const char * svr_name) {
    struct cpe_hash_it svr_info_it;
    set_svr_svr_info_t svr_info;

    cpe_hash_it_init(&svr_info_it, &stub->m_svr_infos);

    while((svr_info = cpe_hash_it_next(&svr_info_it))) {
        if (strcmp(svr_info->m_svr_type_name, svr_name) == 0) return svr_info;
    }

    return NULL;
}

set_svr_svr_info_t set_svr_svr_info_find_by_meta(set_svr_stub_t stub, const char * meta_name) {
    struct cpe_hash_it svr_info_it;
    set_svr_svr_info_t svr_info;

    cpe_hash_it_init(&svr_info_it, &stub->m_svr_infos);

    while((svr_info = cpe_hash_it_next(&svr_info_it))) {
        if (strcmp(dr_meta_name(svr_info->m_pkg_meta), meta_name) == 0) return svr_info;
    }

    return NULL;
}

void set_svr_svr_info_free_all(set_svr_stub_t stub) {
    struct cpe_hash_it svr_info_it;
    set_svr_svr_info_t svr_info;

    cpe_hash_it_init(&svr_info_it, &stub->m_svr_infos);

    svr_info = cpe_hash_it_next(&svr_info_it);
    while (svr_info) {
        set_svr_svr_info_t next = cpe_hash_it_next(&svr_info_it);
        set_svr_svr_info_free(stub, svr_info);
        svr_info = next;
    }
}

uint16_t set_svr_svr_info_svr_type_id(set_svr_svr_info_t svr_info) {
    return svr_info->m_svr_type_id;
}

const char * set_svr_svr_info_svr_type_name(set_svr_svr_info_t svr_info) {
    return svr_info->m_svr_type_name;
}

LPDRMETA set_svr_svr_info_pkg_meta(set_svr_svr_info_t svr_info) {
    return svr_info->m_pkg_meta;
}

LPDRMETAENTRY set_svr_svr_info_pkg_data_entry(set_svr_svr_info_t svr_info) {
    return svr_info->m_pkg_data_entry;
}

LPDRMETAENTRY set_svr_svr_info_pkg_cmd_entry(set_svr_svr_info_t svr_info) {
    return svr_info->m_pkg_cmd_entry;
}

LPDRMETA set_svr_svr_info_carry_meta(set_svr_svr_info_t svr_info) {
    return svr_info->m_carry_meta;
}

LPDRMETA set_svr_svr_info_error_pkg_meta(set_svr_svr_info_t svr_info) {
    return svr_info->m_error_pkg_meta;
}

uint32_t set_svr_svr_info_error_pkg_cmd(set_svr_svr_info_t svr_info) {
    return svr_info->m_error_pkg_cmd;
}

LPDRMETAENTRY set_svr_svr_info_error_pkg_errno_entry(set_svr_svr_info_t svr_info) {
    return svr_info->m_error_pkg_error_entry;
}

LPDRMETA set_svr_svr_info_find_data_meta_by_cmd(set_svr_svr_info_t svr_info, uint32_t cmd) {
    LPDRMETA union_meta;
    LPDRMETAENTRY data_entry;

    if (svr_info->m_pkg_data_entry == NULL) return NULL;

    union_meta = dr_entry_ref_meta(svr_info->m_pkg_data_entry);
    if (union_meta == NULL) return NULL;

    data_entry = dr_meta_find_entry_by_id(union_meta, cmd);
    if (data_entry == NULL) return NULL;

    return dr_entry_ref_meta(data_entry);
}

uint32_t set_svr_svr_info_id_hash(set_svr_svr_info_t svr_info) {
    return svr_info->m_svr_type_id;
}

int set_svr_svr_info_id_eq(set_svr_svr_info_t l, set_svr_svr_info_t r) {
    return l->m_svr_type_id == r->m_svr_type_id;
}

