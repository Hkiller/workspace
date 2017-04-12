#include <assert.h>
#import <Bugtags/Bugtags.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "appsvr_bugtags_backend.h"

int appsvr_bugtags_backend_init(appsvr_bugtags_module_t module) {
    cfg_t bugtags_cfg = cfg_find_cfg(gd_app_cfg(module->m_app), "args.bugtags");
    cfg_t ios_cfg = cfg_find_cfg(bugtags_cfg, "ios");

    BTGInvocationEvent mode_e;
    
    const char * runing_mode = cfg_get_string(bugtags_cfg, "runing-mode", NULL);
    if (runing_mode == NULL) {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: runing-mode not configured");
        return -1;
    }

    if (strcmp(runing_mode, "silent") == 0) {
        mode_e = BTGInvocationEventNone;
    }
    else if (strcmp(runing_mode, "bubble") == 0) {
        mode_e = BTGInvocationEventBubble;
    }
    else if (strcmp(runing_mode, "shake") == 0) {
        mode_e = BTGInvocationEventShake;
    }
    else {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: runing-mode %s unknown, shoud be (silent/bubble/shake)", runing_mode);
        return -1;
    }
    
    const char * app_key = cfg_get_string(ios_cfg, "app-key", NULL);
    if (app_key == NULL) {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: app-key not configured");
        return -1;
    }
    
    [Bugtags startWithAppKey:[NSString stringWithUTF8String: app_key] invocationEvent:mode_e];
    return 0;
}

void appsvr_bugtags_backend_fini(appsvr_bugtags_module_t module) {
}
