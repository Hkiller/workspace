#ifndef APPSVR_STATISTICS_PUSH_ANDROID_H
#define APPSVR_STATISTICS_PUSH_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_bugtags_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_bugtags_backend {
    appsvr_bugtags_module_t m_module;
    plugin_app_env_module_t m_app_env;

    /*jni*/
    jclass m_bugtags_cls;
    jmethodID m_init;
    jmethodID m_on_pause;
    jmethodID m_on_resume;
};
    
#ifdef __cplusplus
}
#endif
    
#endif
