#ifndef APPSVR_STATISTICS_QIHOO_MODULE_H
#define APPSVR_STATISTICS_QIHOO_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/account/appsvr_account_types.h"
#include "appsvr/payment/appsvr_payment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_qihoo_module * appsvr_qihoo_module_t;
typedef struct appsvr_qihoo_backend * appsvr_qihoo_backend_t;

struct appsvr_qihoo_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;

    appsvr_account_module_t m_account_module;
    appsvr_payment_module_t m_payment_module;

	uint8_t m_land_space;
	char * m_background;
	char * m_token;

    appsvr_qihoo_backend_t m_backend;
    
    struct mem_buffer m_dump_buffer;
};

int appsvr_qihoo_set_token(appsvr_qihoo_module_t module, const char * token);
int appsvr_qihoo_set_background(appsvr_qihoo_module_t module, const char * background);

int appsvr_qihoo_backend_init(appsvr_qihoo_module_t module);
void appsvr_qihoo_backend_fini(appsvr_qihoo_module_t module);

#ifdef __cplusplus
}
#endif

#endif
