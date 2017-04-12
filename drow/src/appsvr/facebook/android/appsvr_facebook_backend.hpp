#ifndef APPSVR_STATISTICS_FACEBOOK_ANDROID_H
#define APPSVR_STATISTICS_FACEBOOK_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_facebook_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_facebook_backend {
    appsvr_facebook_module_t m_module;

    /*account*/
    appsvr_account_adapter_t m_account_adapter;

    /*jni*/
    jclass m_manip_cls;
    jmethodID m_manip_init;
    jmethodID m_fini;
    jmethodID m_manip_login;
    jmethodID m_on_activity_result;
};

int appsvr_facebook_jni_init(appsvr_facebook_backend_t backend);
void appsvr_facebook_jni_fini(appsvr_facebook_backend_t backend);

int appsvr_facebook_delegate_init(appsvr_facebook_backend_t backend);
void appsvr_facebook_delegate_fini(appsvr_facebook_backend_t backend);

#ifdef __cplusplus
}
#endif
    
#endif
