#ifndef APPSVR_STATISTICS_WEIXIN_ANDROID_H
#define APPSVR_STATISTICS_WEIXIN_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_weixin_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_weixin_backend {
    appsvr_weixin_module_t m_module;

    /*jni*/
    jclass m_manip_cls;
    jmethodID m_manip_init;
    jmethodID m_manip_fini;
    jmethodID m_manip_login;
};

int appsvr_weixin_jni_init(appsvr_weixin_backend_t backend);
void appsvr_weixin_jni_fini(appsvr_weixin_backend_t backend);    

#ifdef __cplusplus
}
#endif
    
#endif
