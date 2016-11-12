#ifndef GD_POM_MGR_INTERNAL_TYPES_H
#define GD_POM_MGR_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/pom_grp/pom_grp_types.h"
#include "gd/pom_mgr/pom_mgr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pom_manage {
    mem_allocrator_t m_alloc;
    gd_app_context_t m_app;
    mem_allocrator_t m_allc;
    error_monitor_t m_em;

    pom_grp_obj_mgr_t m_obj_mgr;
    void (*m_fini)(pom_grp_obj_mgr_t mgr);

    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
