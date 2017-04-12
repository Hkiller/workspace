#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "plugin/app_env/plugin_app_env_executor.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr_device_backend.hpp"

extern char g_metalib_appsvr_device[];
static struct plugin_app_env_executor_def s_executor_defs[] = {
    { "appsvr_device_start_install", plugin_app_env_executor_oneway, { appsvr_device_start_install } }
} ;

#define APPSVR_DEVICE_GET_FUN(__arg, __fun, __sign) \
    backend-> __arg = env->GetStaticMethodID(backend->m_device_cls, __fun, __sign); \
    assert(backend-> __arg )
    
int appsvr_device_backend_init(appsvr_device_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_device_backend_t backend;

    backend = (appsvr_device_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_device_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_device_backend_init: alloc fail!");
        return -1;
    }

    jclass device_cls = (jclass)env->FindClass("com/drowgames/device/Device");
    if (device_cls == NULL) {
        CPE_ERROR(module->m_em, "appsvr_device_backend_init: get DeviceUtils class fail!");
        mem_free(module->m_alloc, backend);
        return -1;
    }
    
	backend->m_device_cls = (jclass)env->NewGlobalRef(device_cls);
    if (backend->m_device_cls == NULL) {
        CPE_ERROR(module->m_em, "appsvr_device_backend_init: NewGlobalRef DeviceUtils class fail!");
        mem_free(module->m_alloc, backend);
        return -1;
    }

    APPSVR_DEVICE_GET_FUN(m_init, "init", "(J)V");
    APPSVR_DEVICE_GET_FUN(m_start_install, "startInstall", "(Landroid/content/Context;Ljava/lang/String;)V");
    APPSVR_DEVICE_GET_FUN(m_get_device_id, "getDeviceID", "(Landroid/content/Context;)Ljava/lang/String;");
    APPSVR_DEVICE_GET_FUN(m_get_language, "getLanguage", "()Ljava/lang/String;");
    APPSVR_DEVICE_GET_FUN(m_get_device_model, "getDeviceModel", "()Ljava/lang/String;");
    APPSVR_DEVICE_GET_FUN(m_get_cpu_freq, "getCpuFreq", "()I");
    APPSVR_DEVICE_GET_FUN(m_get_memory, "getTotalMemoryKB", "()J");
    APPSVR_DEVICE_GET_FUN(m_get_network_state, "getNetworkState", "(Landroid/content/Context;)I");

    env->CallStaticVoidMethod(backend->m_device_cls, backend->m_init, (jlong)module);
    
    if (plugin_app_env_executor_bulck_create(
            module->m_app_env, (LPDRMETALIB)g_metalib_appsvr_device, module,
            s_executor_defs, (uint8_t)CPE_ARRAY_SIZE(s_executor_defs)) != 0)
    {
        CPE_ERROR(module->m_em, "appsvr_device_backend_init: install executors fail!");
        env->DeleteGlobalRef(backend->m_device_cls);
        mem_free(module->m_alloc, backend);
        return -1;
    }
    
    module->m_backend = backend;
    
    return 0;
}

void appsvr_device_backend_fini(appsvr_device_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_device_backend_t backend = module->m_backend;
    
    assert(backend);

    plugin_app_env_executor_free_by_ctx(module->m_app_env, module);
    
    assert(backend->m_device_cls);
    env->DeleteGlobalRef(backend->m_device_cls);

    mem_free(module->m_alloc, backend);
    module->m_backend = NULL;
}



extern "C" {

JNIEXPORT void JNICALL Java_com_drowgames_device_Device_onNetworkStateChanged(JNIEnv * jni_env, jclass cls, jlong ptr) {
    appsvr_device_module_t module = (appsvr_device_module_t)ptr;

    APPSVR_DEVICE_NETWORK_INFO network_info;
    if (appsvr_device_backend_set_network_state(module, &network_info) == 0) {
        plugin_app_env_send_notification(module->m_app_env,  module->m_meta_network_info, &network_info, sizeof(network_info));
    }
}

}
