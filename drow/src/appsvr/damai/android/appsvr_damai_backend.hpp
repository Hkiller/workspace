#ifndef APPSVR_STATISTICS_DAMAI_ANDROID_H
#define APPSVR_STATISTICS_DAMAI_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_damai_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_damai_backend {
    appsvr_damai_module_t m_module;

    /*account*/
    appsvr_account_adapter_t m_account_adapter;

    /*payment*/
    appsvr_payment_adapter_t m_payment_adapter;

    /*jni*/
    jclass m_manip_cls;
    jmethodID m_manip_init;
	jmethodID m_manip_fini;
    jmethodID m_manip_suspend;
	jmethodID m_manip_resume;
    jmethodID m_manip_login;
	jmethodID m_manip_dopay;
    jmethodID m_manip_set_userinfo;    
};

int appsvr_damai_jni_init(appsvr_damai_backend_t backend);
void appsvr_damai_jni_fini(appsvr_damai_backend_t backend);    

int appsvr_damai_login_init(appsvr_damai_backend_t backend);
void appsvr_damai_login_fini(appsvr_damai_backend_t backend);    

int appsvr_damai_payment_init(appsvr_damai_backend_t backend);
void appsvr_damai_payment_fini(appsvr_damai_backend_t backend);    

#ifdef __cplusplus
}
#endif
    
#endif
