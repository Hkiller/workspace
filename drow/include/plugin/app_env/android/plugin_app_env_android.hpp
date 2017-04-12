#ifndef PLUGIN_APP_ENV_ANDROID_H
#define PLUGIN_APP_ENV_ANDROID_H
#include <jni.h>
#include "../plugin_app_env_types.h"

extern "C" {

/*log operations*/
void android_cpe_error_log_to_log(struct error_info * info, void * context, const char * fmt, va_list args);

void android_set_current_apk(const char * full_apk_name);
const char * android_current_apk(void);

void android_set_external_dir(const char * extern_dir);
const char * android_external_dir(void);
void android_set_internal_dir(const char * dir_dir);    
const char * android_internal_dir(void);

void android_set_jvm(void * jvm);
void * android_jvm(void);
JNIEnv * android_jni_env(void);

int plugin_app_env_android_set_activity(plugin_app_env_module_t module, void * activity);    
void * plugin_app_env_android_activity(plugin_app_env_module_t module);

jclass plugin_app_env_android_find_class(plugin_app_env_module_t module, const char * class_name);

uint8_t plugin_app_env_android_check_exception(error_monitor_t em);
char * plugin_app_env_android_dup_str(char * buf, size_t buf_size, jstring jstr);

/*delegate */
typedef void (*plugin_app_env_android_on_activity_result_fun_t)(void * ctx, void * intent, int requestCode, int resultCode);
typedef void (*plugin_app_env_android_on_new_intent_fun_t)(void * ctx, void * intent);

int plugin_app_env_android_register_delegate(
    plugin_app_env_module_t app_env,
    void * ctx,
    plugin_app_env_android_on_new_intent_fun_t new_intent,
    plugin_app_env_android_on_activity_result_fun_t on_activity_result);

void plugin_app_env_android_unregister_delegate(
    plugin_app_env_module_t app_env,
    void * ctx);

void plugin_app_env_android_dispatch_new_intent(plugin_app_env_module_t app_env, void * intent);
void plugin_app_env_android_dispatch_activity_result(plugin_app_env_module_t app_env, void * intent, int requestCode, int resultCode);
    
}

#endif

