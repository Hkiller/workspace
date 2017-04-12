#ifndef APPSVR_STATISTICS_googlepay_ANDROID_H
#define APPSVR_STATISTICS_googlepay_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_googlepay_module_i.h"
#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_googlepay_backend {
    appsvr_googlepay_module_t m_module;

    /*jni*/
    jclass m_manip_cls;
    jmethodID m_manip_init;
	jmethodID m_manip_dopay;
	jmethodID m_fini;
    jmethodID m_on_activity_result;
    jmethodID m_do_sync_products;
};

int appsvr_googlepay_jni_init(appsvr_googlepay_backend_t backend);
void appsvr_googlepay_jni_fini(appsvr_googlepay_backend_t backend);    

int appsvr_googlepay_delegate_init(appsvr_googlepay_backend_t backend);
void appsvr_googlepay_delegate_fini(appsvr_googlepay_backend_t backend);    
#ifdef __cplusplus
}
#endif
    
#endif
