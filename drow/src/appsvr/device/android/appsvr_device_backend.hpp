#ifndef APPSVR_STATISTICS_DEVICE_ANDROID_H
#define APPSVR_STATISTICS_DEVICE_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_device_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_device_backend {
    /*jni*/
    jclass m_device_cls;
    jmethodID m_init;    
    jmethodID m_start_install;
    jmethodID m_get_device_id;
    jmethodID m_get_language;
    jmethodID m_get_device_model;
    jmethodID m_get_cpu_freq;
    jmethodID m_get_memory;
    jmethodID m_get_network_state;
};

/*executors*/
void appsvr_device_start_install(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size);
    
#ifdef __cplusplus
}
#endif
    
#endif
