#ifndef APPSVR_STATISTICS_yomobads_ANDROID_H
#define APPSVR_STATISTICS_yomobads_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_yomobads_module_i.h"
#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_yomobads_backend {
    appsvr_yomobads_module_t m_module;

    /*jni*/
    jclass m_manip_cls;
    jmethodID m_manip_init;
	jmethodID m_manip_do_adsopen;
	jmethodID m_fini;
    jmethodID m_on_suspend;
    jmethodID m_on_resume;
    jmethodID m_on_pause;
    jmethodID m_on_activity_result;
    jmethodID m_on_start;
    jmethodID m_on_destroy;
};

int appsvr_yomobads_jni_init(appsvr_yomobads_backend_t backend);
void appsvr_yomobads_jni_fini(appsvr_yomobads_backend_t backend);
#ifdef __cplusplus
}
#endif
    
#endif
