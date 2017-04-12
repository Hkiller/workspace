#ifndef UI_APP_MANIP_BUILD_PHASE_SRCS_I_H
#define UI_APP_MANIP_BUILD_PHASE_SRCS_I_H
#include "convert_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

struct convert_pack_phase {
    convert_ctx_t m_ctx;
    TAILQ_ENTRY(convert_pack_phase) m_next;
    
    char m_name[64];
    plugin_package_package_t m_package;
};

convert_pack_phase_t convert_pack_phase_create(convert_ctx_t ctx, const char * phase_name);
void convert_pack_phase_free(convert_pack_phase_t phase);

convert_pack_phase_t convert_pack_phase_find(convert_ctx_t ctx, const char * phase_name);
    
convert_pack_phase_t
convert_pack_phase_check_create(convert_ctx_t ctx, const char * phase_name);

#ifdef __cplusplus
}
#endif

#endif
