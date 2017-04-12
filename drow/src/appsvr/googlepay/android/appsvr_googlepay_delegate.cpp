#include "appsvr_googlepay_backend.hpp"

static void appsvr_googlepay_on_activity_result(void * ctx, void * intent, int requestCode, int resultCode) {
    appsvr_googlepay_module_t module = (appsvr_googlepay_module_t)ctx;
    
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_activity_result,
        jobject(intent),
        (jint)requestCode,
        (jint)resultCode);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_googlepay_on_activity_result: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_googlepay_on_activity_result: enter!");
}

int appsvr_googlepay_delegate_init(appsvr_googlepay_backend_t backend) {
    if (plugin_app_env_android_register_delegate(
            backend->m_module->m_app_env,
            backend->m_module,
            NULL,
            appsvr_googlepay_on_activity_result) != 0)
    {
        CPE_ERROR(backend->m_module->m_em, "appsvr_googlepay_delegate_init: register fail!");
        return -1;
    }

    return 0;
}

void appsvr_googlepay_delegate_fini(appsvr_googlepay_backend_t backend) {
    plugin_app_env_android_unregister_delegate(backend->m_module->m_app_env, backend->m_module);
}



