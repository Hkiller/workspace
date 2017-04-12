#ifndef PLUGIN_APP_ENV_ANDROID_BACKEND_I_H
#define PLUGIN_APP_ENV_ANDROID_BACKEND_I_H
#include <jni.h>
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../plugin_app_env_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_app_env_delegator * plugin_app_env_delegator_t;
typedef TAILQ_HEAD(plugin_app_env_delegator_list,  plugin_app_env_delegator) plugin_app_env_delegator_list_t;
    
struct plugin_app_env_backend {
    plugin_app_env_module_t m_module;
    plugin_app_env_delegator_list_t m_delegators;
    jobject m_activity;
    
    jobject m_dex_loader;
    jmethodID m_find_class_mid;
};
    
#ifdef __cplusplus
}
#endif

#endif
