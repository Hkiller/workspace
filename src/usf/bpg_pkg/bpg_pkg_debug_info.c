#include "bpg_pkg_internal_ops.h"

uint32_t bpg_pkg_debug_info_hash(const struct bpg_pkg_debug_info * o) {
    return o->m_cmd;
}

int bpg_pkg_debug_info_cmp(const struct bpg_pkg_debug_info * l, const struct bpg_pkg_debug_info * r) {
    return l->m_cmd == r->m_cmd;
}

struct bpg_pkg_debug_info *
bpg_pkg_debug_info_create(bpg_pkg_manage_t mgr, uint32_t cmd) {
    struct bpg_pkg_debug_info * info;

    info = mem_alloc(mgr->m_alloc, sizeof(struct bpg_pkg_debug_info));
    if (info == NULL) return NULL;

    info->m_cmd = cmd;
    info->m_debug_level = bpg_pkg_debug_none;

    cpe_hash_entry_init(&info->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_pkg_debug_infos, info) != 0) {
        mem_free(mgr->m_alloc, info);
        return NULL;
    }

    return info;
}

void bpg_pkg_debug_info_free(bpg_pkg_manage_t mgr, struct bpg_pkg_debug_info * o) {
    cpe_hash_table_remove_by_ins(&mgr->m_pkg_debug_infos, o);
    mem_free(mgr->m_alloc, o);
}

void bpg_pkg_debug_info_free_all(bpg_pkg_manage_t mgr) {
    struct cpe_hash_it debug_info_it;
    struct bpg_pkg_debug_info * debug_info;

    cpe_hash_it_init(&debug_info_it, &mgr->m_pkg_debug_infos);

    debug_info = cpe_hash_it_next(&debug_info_it);
    while (debug_info) {
        struct bpg_pkg_debug_info * next = cpe_hash_it_next(&debug_info_it);
        bpg_pkg_debug_info_free(mgr, debug_info);
        debug_info = next;
    }
}

bpg_pkg_debug_level_t bpg_pkg_manage_debug_level(bpg_pkg_manage_t mgr, uint32_t cmd) {
    struct bpg_pkg_debug_info key;
    struct bpg_pkg_debug_info * info;

    key.m_cmd = cmd;

    info = cpe_hash_table_find(&mgr->m_pkg_debug_infos, &key);
    
    return info ? info->m_debug_level : mgr->m_pkg_debug_default_level;
}

void bpg_pkg_manage_set_debug_level(bpg_pkg_manage_t mgr, uint32_t cmd, bpg_pkg_debug_level_t level) {
    struct bpg_pkg_debug_info key;
    struct bpg_pkg_debug_info * info;

    key.m_cmd = cmd;
    info = cpe_hash_table_find(&mgr->m_pkg_debug_infos, &key);
    if (info == NULL) info = bpg_pkg_debug_info_create(mgr, cmd);
    if (info) info->m_debug_level = level;
}
