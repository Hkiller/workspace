#include "cpe/dr/dr_metalib_manage.h"
#include "bpg_pkg_internal_ops.h"

uint32_t bpg_pkg_cmd_info_cmd_hash(const struct bpg_pkg_cmd_info * o) {
    return o->m_cmd;
}

uint32_t bpg_pkg_cmd_info_name_hash(const struct bpg_pkg_cmd_info * o) {
    return cpe_hash_str(o->m_name, strlen(o->m_name));
}

int bpg_pkg_cmd_info_cmd_eq(const struct bpg_pkg_cmd_info * l, const struct bpg_pkg_cmd_info * r) {
    return l->m_cmd == r->m_cmd;
}

int bpg_pkg_cmd_info_name_eq(const struct bpg_pkg_cmd_info * l, const struct bpg_pkg_cmd_info * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

struct bpg_pkg_cmd_info *
bpg_pkg_cmd_info_create(bpg_pkg_manage_t mgr, uint32_t cmd, LPDRMETA meta) {
    struct bpg_pkg_cmd_info * info;

    info = mem_alloc(mgr->m_alloc, sizeof(struct bpg_pkg_cmd_info));
    if (info == NULL) return NULL;

    info->m_cmd = cmd;
    info->m_name = dr_meta_name(meta);
    info->m_cmd_meta = meta;

    cpe_hash_entry_init(&info->m_hh_for_cmd);
    if (cpe_hash_table_insert_unique(&mgr->m_cmd_info_by_cmd, info) != 0) {
        mem_free(mgr->m_alloc, info);
        return NULL;
    }

    cpe_hash_entry_init(&info->m_hh_for_name);
    if (cpe_hash_table_insert_unique(&mgr->m_cmd_info_by_name, info) != 0) {
        cpe_hash_table_remove_by_ins(&mgr->m_cmd_info_by_cmd, info);
        mem_free(mgr->m_alloc, info);
        return NULL;
    }

    return info;
}

void bpg_pkg_cmd_info_free(bpg_pkg_manage_t mgr, struct bpg_pkg_cmd_info * o) {
    cpe_hash_table_remove_by_ins(&mgr->m_cmd_info_by_cmd, o);
    cpe_hash_table_remove_by_ins(&mgr->m_cmd_info_by_name, o);
    mem_free(mgr->m_alloc, o);
}

void bpg_pkg_cmd_info_free_all(bpg_pkg_manage_t mgr) {
    struct cpe_hash_it cmd_info_it;
    struct bpg_pkg_cmd_info * cmd_info;

    cpe_hash_it_init(&cmd_info_it, &mgr->m_cmd_info_by_cmd);

    cmd_info = cpe_hash_it_next(&cmd_info_it);
    while (cmd_info) {
        struct bpg_pkg_cmd_info * next = cpe_hash_it_next(&cmd_info_it);
        bpg_pkg_cmd_info_free(mgr, cmd_info);
        cmd_info = next;
    }
}

