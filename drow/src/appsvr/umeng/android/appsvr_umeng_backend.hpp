#ifndef APPSVR_STATISTICS_UMENG_ANDROID_H
#define APPSVR_STATISTICS_UMENG_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_umeng_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_umeng_backend {
    appsvr_umeng_module_t m_module;

    /*manip*/
    jclass m_manip_cls;
    jmethodID m_manip_init;

    /*agent*/
    jclass m_agent_cls;
    jmethodID m_agent_report_error;
    jmethodID m_agent_set_page_track;
    jmethodID m_agent_on_page_start;
    jmethodID m_agent_on_page_end;
    jmethodID m_agent_on_pause;
    jmethodID m_agent_on_resume;
    jmethodID m_agent_on_kill;
    jmethodID m_agent_on_event;
    jmethodID m_agent_set_debug;

    /*map*/
    jclass m_map_cls;
    jmethodID m_map_init;
    jmethodID m_map_put;
};

int appsvr_umeng_jni_init(appsvr_umeng_backend_t backend);
void appsvr_umeng_jni_fini(appsvr_umeng_backend_t backend);

int appsvr_umeng_agent_init(appsvr_umeng_backend_t backend);
void appsvr_umeng_agent_fini(appsvr_umeng_backend_t backend);

int appsvr_umeng_crash_reporter_init(appsvr_umeng_backend_t backend);
void appsvr_umeng_crash_reporter_fini(appsvr_umeng_backend_t backend);

int appsvr_umeng_jni_load_method(
    appsvr_umeng_backend_t backend, JNIEnv* env, jmethodID & id,
    bool isstatic, const jclass claxx, const char* methodname, const char* methodsigner);
    
#ifdef __cplusplus
}
#endif
    
#endif
