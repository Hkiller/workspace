#include "cpe/utils/string_utils.h"
#include "appsvr_facebook_permission_i.h"

appsvr_facebook_permission_t
appsvr_facebook_permission_create(appsvr_facebook_module_t module, const char * str_permission) {
    appsvr_facebook_permission_t permission;

    permission = mem_alloc(module->m_alloc, sizeof(struct appsvr_facebook_permission));
    if (permission == NULL) {
        CPE_ERROR(module->m_em, "appsvr_facebook_permission_create: alloc fail!");
        return NULL;
    }

    permission->m_module = module;
    cpe_str_dup(permission->m_permission, sizeof(permission->m_permission), str_permission);
    permission->m_is_gaint = 0;
    
    TAILQ_INSERT_TAIL(&module->m_permissions, permission, m_next);
    return permission;
}

void appsvr_facebook_permission_free(appsvr_facebook_permission_t permission) {
    appsvr_facebook_module_t module = permission->m_module;

    TAILQ_REMOVE(&module->m_permissions, permission, m_next);
    
    mem_free(module->m_alloc, permission);
}

