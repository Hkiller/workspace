#ifndef APPSVR_NOTIFY_TAG_H
#define APPSVR_NOTIFY_TAG_H
#include "appsvr_notify_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_notify_tag_t appsvr_notify_tag_create(appsvr_notify_module_t module, const char * name);
appsvr_notify_tag_t appsvr_notify_tag_check_create(appsvr_notify_module_t module, const char * name);
void appsvr_notify_tag_free(appsvr_notify_tag_t tag);
    
appsvr_notify_tag_t
appsvr_notify_tag_find(appsvr_notify_module_t module, const char * name);

void appsvr_notify_tag_clear(appsvr_notify_tag_t tag);

#ifdef __cplusplus
}
#endif

#endif
