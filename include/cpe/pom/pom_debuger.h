#ifndef CPE_POM_DEBUGER_H
#define CPE_POM_DEBUGER_H
#include "cpe/utils/error.h"
#include "pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_debuger_t
pom_debuger_enable(pom_mgr_t mgr, uint32_t stack_size, error_monitor_t em);
pom_debuger_t pom_debuger_get(pom_mgr_t mgr);

error_monitor_t pom_debuger_em(pom_mgr_t mgr);
int pom_debuger_validate(pom_debuger_t debuger, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
