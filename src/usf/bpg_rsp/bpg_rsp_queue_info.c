#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "bpg_rsp_internal_ops.h"

struct bpg_rsp_queue_info *
bpg_rsp_queue_info_create(bpg_rsp_manage_t mgr, const char * queue_name, bpg_rsp_queue_scope_t scope, uint32_t max_count) {
    char * buf;
    struct bpg_rsp_queue_info * queue_info;
    size_t name_len = cpe_hs_len_to_binary_len(strlen(queue_name));
    CPE_PAL_ALIGN_DFT(name_len);

    buf = mem_alloc(mgr->m_alloc, sizeof(struct bpg_rsp_queue_info) + name_len);
    if (buf == NULL) return NULL;

    cpe_hs_init((cpe_hash_string_t)buf, name_len, queue_name);

    queue_info = (struct bpg_rsp_queue_info*)(buf + name_len);
    queue_info->m_name = (cpe_hash_string_t)buf;
    queue_info->m_scope = scope;
    queue_info->m_max_count = max_count;

    cpe_hash_entry_init(&queue_info->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_queue_infos, queue_info) != 0) {
        mem_free(mgr->m_alloc, buf);
        return NULL;
    }

    return queue_info;
}

void bpg_rsp_queue_info_free(
    bpg_rsp_manage_t mgr,
    struct bpg_rsp_queue_info * queue_info)
{
    cpe_hash_table_remove_by_ins(&mgr->m_queue_infos, queue_info);
    mem_free(mgr->m_alloc, queue_info->m_name);
}

void bpg_rsp_queue_info_free_all(bpg_rsp_manage_t mgr) {
    struct cpe_hash_it queue_it;
    struct bpg_rsp_queue_info * queue;

    cpe_hash_it_init(&queue_it, &mgr->m_queue_infos);

    queue = cpe_hash_it_next(&queue_it);
    while (queue) {
        struct bpg_rsp_queue_info * next = cpe_hash_it_next(&queue_it);
        bpg_rsp_queue_info_free(mgr, queue);
        queue = next;
    }
}

const char * bpg_rsp_queue_name(const struct bpg_rsp_queue_info * queue) {
    return cpe_hs_data(queue->m_name);
}

uint32_t bpg_rsp_queue_info_hash(const struct bpg_rsp_queue_info * queue) {
    return cpe_hs_value(queue->m_name);
}

int bpg_rsp_queue_info_cmp(const struct bpg_rsp_queue_info * l, const struct bpg_rsp_queue_info * r) {
    return cpe_hs_cmp(l->m_name, r->m_name) == 0;
}

struct bpg_rsp_queue_info *
bpg_rsp_queue_info_find(bpg_rsp_manage_t mgr, cpe_hash_string_t queue_name) {
    struct bpg_rsp_queue_info key;
    key.m_name = queue_name;
    return (struct bpg_rsp_queue_info *)cpe_hash_table_find(&mgr->m_queue_infos, &key);
}
