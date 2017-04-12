#ifndef APPSVR_STATISTICS_cmccpay_ANDROID_H
#define APPSVR_STATISTICS_cmccpay_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_cmccpay_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_cmccpay_backend {
    appsvr_cmccpay_module_t m_module;

    /*account*/
    appsvr_account_adapter_t m_account_adapter;

    /*payment*/
    appsvr_payment_adapter_t m_payment_adapter;
    
    /*jni*/
    jclass m_manip_cls;
    jmethodID m_manip_init;
	jmethodID m_manip_dopay;
	jmethodID m_fini;
    jmethodID m_exitGame;
    jmethodID m_moreGames;
};

int appsvr_cmccpay_jni_init(appsvr_cmccpay_backend_t backend);
void appsvr_cmccpay_jni_fini(appsvr_cmccpay_backend_t backend);    

int appsvr_cmccpay_payment_init(appsvr_cmccpay_backend_t backend);
void appsvr_cmccpay_payment_fini(appsvr_cmccpay_backend_t backend);    

#ifdef __cplusplus
}
#endif
    
#endif
