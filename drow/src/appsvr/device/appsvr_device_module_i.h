#ifndef APPSVR_STATISTICS_DEVICE_MODULE_H
#define APPSVR_STATISTICS_DEVICE_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "protocol/appsvr/device/appsvr_device_pro.h"
#include "appsvr/device/appsvr_device_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_device_module * appsvr_device_module_t;
typedef struct appsvr_device_backend * appsvr_device_backend_t;

struct appsvr_device_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;
    appsvr_device_backend_t m_backend;

    LPDRMETA m_meta_device_info;
    LPDRMETA m_meta_path_info;
    LPDRMETA m_meta_network_info;    

    struct mem_buffer m_dump_buffer;
};

int appsvr_device_backend_init(appsvr_device_module_t module);
void appsvr_device_backend_fini(appsvr_device_module_t module);

/*backend*/
int appsvr_device_backend_set_network_state(appsvr_device_module_t module, APPSVR_DEVICE_NETWORK_INFO * network_info);
void appsvr_device_backend_set_device_info(appsvr_device_module_t module, APPSVR_DEVICE_INFO * device_info);
void appsvr_device_backend_set_path_info(appsvr_device_module_t module, APPSVR_DEVICE_QUERY_PATH const * req, APPSVR_DEVICE_PATH * path_info);

#ifdef __cplusplus
}
#endif

#endif
