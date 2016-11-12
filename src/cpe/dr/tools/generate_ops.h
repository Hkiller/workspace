#ifndef CPE_DR_TOOLS_GENERATE_OPS_H
#define CPE_DR_TOOLS_GENERATE_OPS_H
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cpe_dr_generate_ctx {
    dr_metalib_builder_t m_builder;
    LPDRMETALIB m_metalib;
    error_monitor_t m_em;
} * cpe_dr_generate_ctx_t;

int cpe_dr_generate_h(write_stream_t stream, dr_metalib_source_t source, int with_traits, cpe_dr_generate_ctx_t ctx);
int cpe_dr_generate_lib_c(write_stream_t stream, const char * arg_name, cpe_dr_generate_ctx_t ctx);
int cpe_dr_generate_traits_cpp(write_stream_t stream, const char * arg_name, cpe_dr_generate_ctx_t ctx);

#ifdef __cplusplus
}
#endif

#endif
