#ifndef UI_MODEL_TOOL_CONVERT_CTX_RES_TO_SRC_H
#define UI_MODEL_TOOL_CONVERT_CTX_RES_TO_SRC_H
#include "convert_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

struct convert_ctx_res_to_src {
    struct cpe_hash_entry m_hh;
    ui_cache_res_t m_res;
    ui_data_src_t m_src;
};

convert_ctx_res_to_src_t
convert_ctx_res_to_src_check_create(convert_ctx_t ctx, ui_cache_res_t res, ui_data_src_t src);
void convert_ctx_res_to_src_free(convert_ctx_t ctx, convert_ctx_res_to_src_t res_to_src);

void convert_ctx_res_to_src_free_all(convert_ctx_t ctx);

uint32_t convert_ctx_res_to_src_hash(convert_ctx_res_to_src_t src);
int convert_ctx_res_to_src_eq(convert_ctx_res_to_src_t l, convert_ctx_res_to_src_t r);
    
#ifdef __cplusplus
}
#endif

#endif
