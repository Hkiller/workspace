#include "convert_ctx_res_to_src.h"

convert_ctx_res_to_src_t
convert_ctx_res_to_src_check_create(convert_ctx_t ctx, ui_cache_res_t res, ui_data_src_t src) {
    struct convert_ctx_res_to_src key;
    convert_ctx_res_to_src_t res_to_src;
    
    key.m_res = res;

    for(res_to_src = cpe_hash_table_find(&ctx->m_res_to_srcs, &key);
        res_to_src;
        res_to_src = cpe_hash_table_find_next(&ctx->m_res_to_srcs, res_to_src))
    {
        if (res_to_src->m_src == src) return res_to_src;
    }

    res_to_src = mem_alloc(ctx->m_alloc, sizeof(struct convert_ctx_res_to_src));
    if (res_to_src == NULL) {
        CPE_ERROR(ctx->m_em, "convert_ctx_res_to_src: alloc fail!");
        return NULL;
    }

    res_to_src->m_res = res;
    res_to_src->m_src = src;

    cpe_hash_entry_init(&res_to_src->m_hh);
    if (cpe_hash_table_insert(&ctx->m_res_to_srcs, res_to_src) != 0) {
        CPE_ERROR(ctx->m_em, "convert_ctx_res_to_src: insert fail!");
        mem_free(ctx->m_alloc, res_to_src);
        return NULL;
    }

    return res_to_src;
}

void convert_ctx_res_to_src_free(convert_ctx_t ctx, convert_ctx_res_to_src_t res_to_src) {
    cpe_hash_table_remove_by_ins(&ctx->m_res_to_srcs, res_to_src);
    mem_free(ctx->m_alloc, res_to_src);
}

void convert_ctx_res_to_src_free_all(convert_ctx_t ctx) {
    struct cpe_hash_it res_to_src_it;
    convert_ctx_res_to_src_t res_to_src;

    cpe_hash_it_init(&res_to_src_it, &ctx->m_res_to_srcs);

    res_to_src = cpe_hash_it_next(&res_to_src_it);
    while (res_to_src) {
        convert_ctx_res_to_src_t next = cpe_hash_it_next(&res_to_src_it);
        convert_ctx_res_to_src_free(ctx, res_to_src);
        res_to_src = next;
    }
}

uint32_t convert_ctx_res_to_src_hash(convert_ctx_res_to_src_t src) {
    return (uint32_t)((ptr_int_t)(src->m_res));
}

int convert_ctx_res_to_src_eq(convert_ctx_res_to_src_t l, convert_ctx_res_to_src_t r) {
    return l->m_res == r->m_res ? 1 : 0;
}



