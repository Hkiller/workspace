#ifndef APPSVR_STATISTICS_HAOXIN_ANDROID_H
#define APPSVR_STATISTICS_HAOXIN_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_haoxin_module_i.h"
#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_haoxin_backend {
    appsvr_haoxin_module_t m_module;

    /*jni*/
    jclass m_manip_cls;
    jmethodID m_manip_init;
	jmethodID m_manip_dopay;
	jmethodID m_fini;
    jmethodID m_exitGame;
    jmethodID m_moreGames;
    jmethodID m_sdkExitGameSupport;
    jmethodID m_sdkMoreGameSupport;
    jmethodID m_on_suspend;
    jmethodID m_on_resume;
    jmethodID m_on_pause;
    jmethodID m_on_activity_result;
    jmethodID m_on_new_intent;
};

int appsvr_haoxin_jni_init(appsvr_haoxin_backend_t backend);
void appsvr_haoxin_jni_fini(appsvr_haoxin_backend_t backend);

int appsvr_haoxin_delegate_init(appsvr_haoxin_backend_t backend);
void appsvr_haoxin_delegate_fini(appsvr_haoxin_backend_t backend);
    
int appsvr_haoxin_sdk_action_monitor_init(appsvr_haoxin_backend_t backend);
int appsvr_haoxin_sdk_action_monitor_fini(appsvr_haoxin_backend_t backend);

#ifdef __cplusplus
}
#endif
    
#endif
