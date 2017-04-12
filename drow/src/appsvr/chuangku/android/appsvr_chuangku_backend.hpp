#ifndef APPSVR_STATISTICS_chuangku_ANDROID_H
#define APPSVR_STATISTICS_chuangku_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_chuangku_module_i.h"
#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_chuangku_backend {
    appsvr_chuangku_module_t m_module;

    /*jni*/
    jclass m_manip_cls;
    jmethodID m_manip_init;
	jmethodID m_fini;
    jmethodID m_manip_dopay;
    jmethodID m_getMoreGameIsSupport;
//    jmethodID m_show_exit_page;
//    jmethodID m_show_more_games_page;
    jmethodID m_on_get_addition_attr;
    jmethodID m_on_suspend;
    jmethodID m_on_resume;
    jmethodID m_on_activity_result;
    jmethodID m_on_new_intent;
    jmethodID m_on_call_action ;
};

int appsvr_chuangku_jni_init(appsvr_chuangku_backend_t backend);
void appsvr_chuangku_jni_fini(appsvr_chuangku_backend_t backend);    

int appsvr_chuangku_delegate_init(appsvr_chuangku_backend_t backend);
void appsvr_chuangku_delegate_fini(appsvr_chuangku_backend_t backend);    
    
/*sdkÒ³Ãæ½Ó¿Ú */
int appsvr_chuangku_sdk_action_monitor_init(appsvr_chuangku_backend_t backend);
void appsvr_chuangku_sdk_action_monitor_fini(appsvr_chuangku_backend_t backend);

#ifdef __cplusplus
}
#endif
    
#endif
