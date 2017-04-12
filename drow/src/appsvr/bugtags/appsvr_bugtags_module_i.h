#ifndef APPSVR_STATISTICS_PUSH_MODULE_H
#define APPSVR_STATISTICS_PUSH_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_bugtags_module * appsvr_bugtags_module_t;
typedef struct appsvr_bugtags_backend * appsvr_bugtags_backend_t;

struct appsvr_bugtags_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;
    appsvr_bugtags_backend_t m_backend;
};

int appsvr_bugtags_backend_init(appsvr_bugtags_module_t module);
void appsvr_bugtags_backend_fini(appsvr_bugtags_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
