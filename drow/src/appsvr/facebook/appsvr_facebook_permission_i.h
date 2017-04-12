#ifndef APPSVR_STATISTICS_FACEBOOK_PERMISSION_H
#define APPSVR_STATISTICS_FACEBOOK_PERMISSION_H
#include "appsvr_facebook_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_facebook_permission {
    appsvr_facebook_module_t m_module;
    TAILQ_ENTRY(appsvr_facebook_permission) m_next;
    char m_permission[64];
    uint8_t m_is_gaint;
};

appsvr_facebook_permission_t appsvr_facebook_permission_create(appsvr_facebook_module_t module, const char * permission);
void appsvr_facebook_permission_free(appsvr_facebook_permission_t prmission);
    
#ifdef __cplusplus
}
#endif

#endif
