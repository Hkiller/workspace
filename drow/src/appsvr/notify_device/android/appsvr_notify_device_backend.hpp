#ifndef APPSVR_STATISTICS_PUSH_ANDROID_H
#define APPSVR_STATISTICS_PUSH_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_notify_device_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_notify_device_backend {
    appsvr_notify_device_module_t m_module;

    /*jni*/
    jclass m_notify_device_mgr_cls;
    jmethodID m_alarm_add;
    jmethodID m_alarm_remove;
};
    
#ifdef __cplusplus
}
#endif
    
#endif
