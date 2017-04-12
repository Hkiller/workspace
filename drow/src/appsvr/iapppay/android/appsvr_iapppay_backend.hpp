#ifndef APPSVR_IAPPPAY_ANDROID_H
#define APPSVR_IAPPPAY_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_iapppay_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_iapppay_backend {
    appsvr_iapppay_module_t m_adapter;
    plugin_app_env_module_t m_app_env;
    
    /*jni*/
    jclass m_iapppay_cls;
    jmethodID m_init;
    jmethodID m_fini;
    jmethodID m_start_pay;
};
    
#ifdef __cplusplus
}
#endif
    
#endif
