#ifndef APPSVR_ACCOUNT_MODULE_I_H
#define APPSVR_ACCOUNT_MODULE_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr/package/appsvr_package_module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_package_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
