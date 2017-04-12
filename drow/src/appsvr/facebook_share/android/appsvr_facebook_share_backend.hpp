#ifndef APPSVR_STATISTICS_facebook_share_ANDROID_H
#define APPSVR_STATISTICS_facebook_share_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_facebook_share_module_i.h"
#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_facebook_share_backend {
    appsvr_facebook_share_module_t m_module;

    jclass m_manip_cls;
    jmethodID m_init;
    jmethodID m_fini;
    jmethodID m_onShare;
    jmethodID m_on_activity_result;
};

int appsvr_facebook_share_jni_init(appsvr_facebook_share_backend_t backend);
void appsvr_facebook_share_jni_fini(appsvr_facebook_share_backend_t backend);

int appsvr_facebook_share_delegate_init(appsvr_facebook_share_backend_t backend);
void appsvr_facebook_share_delegate_fini(appsvr_facebook_share_backend_t backend);

#ifdef __cplusplus
}
#endif
    
#endif
