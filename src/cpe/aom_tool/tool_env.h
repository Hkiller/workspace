#ifndef CPE_AOM_TOOLS_ENV_H
#define CPE_AOM_TOOLS_ENV_H
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aom_tool_env {
    LPDRMETALIB m_metalib;
    LPDRMETA m_meta;
    error_monitor_t m_em;
    struct mem_buffer m_input_meta_buffer;
} * cpe_dr_tool_env_t;

int aom_tool_generate_hpp(struct aom_tool_env * env, const char * filename, const char * classname, const char * namespace);

#ifdef __cplusplus
}
#endif

#endif
