#include <assert.h>
#include "logic_internal_ops.h"

logic_executor_ref_t
logic_executor_ref_create(logic_executor_mgr_t mgr, const char * name, logic_executor_t executor) {
    char * buf;
    logic_executor_ref_t executor_ref;
    size_t name_len;

    name_len = strlen(name) + 1;

    buf = mem_alloc(mgr->m_alloc, sizeof(struct logic_executor_ref) + name_len);
    if (buf == NULL) return NULL;

    memcpy(buf, name, name_len);

    executor_ref = (logic_executor_ref_t)(buf + name_len);

    executor_ref->m_mgr = mgr;
    executor_ref->m_ref_count = 1;
    executor_ref->m_name = buf;
    executor_ref->m_executor = executor;

    cpe_hash_entry_init(&executor_ref->m_hh);

    if (cpe_hash_table_insert_unique(&mgr->m_executor_refs, executor_ref) != 0) {
        mem_free(mgr->m_alloc, buf);
        return NULL;
    }

    return executor_ref;
}

void logic_executor_ref_free(logic_executor_ref_t executor_ref) {
    logic_executor_mgr_t mgr = executor_ref->m_mgr;
    assert(executor_ref->m_ref_count == 0);
    cpe_hash_table_remove_by_ins(&mgr->m_executor_refs, executor_ref);
    mem_free(mgr->m_alloc, (void*)executor_ref->m_name);
}

void logic_executor_ref_inc(logic_executor_ref_t executor_ref) {
    ++executor_ref->m_ref_count;
}

void logic_executor_ref_dec(logic_executor_ref_t executor_ref) {
    --executor_ref->m_ref_count;
    if (executor_ref->m_ref_count == 0) {
        logic_executor_ref_free(executor_ref);
    }
}

logic_executor_t
logic_executor_ref_executor(logic_executor_ref_t ref) {
    return ref->m_executor;
}

logic_executor_ref_t
logic_executor_ref_find(logic_executor_mgr_t mgr, const char * name) {
    struct logic_executor_ref key;
    key.m_name = name;

    return (logic_executor_ref_t)cpe_hash_table_find(&mgr->m_executor_refs, &key);
}

uint32_t logic_executor_ref_hash(const struct logic_executor_ref * type) {
    return cpe_hash_str(type->m_name, strlen(type->m_name));
}

int logic_executor_ref_cmp(const struct logic_executor_ref * l, const struct logic_executor_ref * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void logic_executor_ref_free_all(logic_executor_mgr_t mgr) {
    struct cpe_hash_it ref_it;
    logic_executor_ref_t ref;

    cpe_hash_it_init(&ref_it, &mgr->m_executor_refs);

    ref = cpe_hash_it_next(&ref_it);
    while(ref) {
        logic_executor_ref_t next = cpe_hash_it_next(&ref_it);
        logic_executor_ref_free(ref);
        ref = next;
    }
}

