#ifndef APPSVR_IAPPPAY_ANDROID_H
#define APPSVR_IAPPPAY_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_unicompay_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_unicompay_backend {
    appsvr_unicompay_module_t m_adapter;
    plugin_app_env_module_t m_app_env;
    
    /*jni*/
    jclass m_unicompay_cls;
    jmethodID m_init;
    jmethodID m_fini;
    jmethodID m_start_pay_offline;
};
    
#ifdef __cplusplus
}
#endif
    
#endif
