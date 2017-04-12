#include "appsvr_haoxin_backend.hpp"

static void appsvr_haoxin_on_activity_result(void * ctx, void * intent, int requestCode, int resultCode) {
    appsvr_haoxin_module_t module = (appsvr_haoxin_module_t)ctx;
    
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_activity_result,
        jobject(intent),
        (jint)requestCode,
        (jint)resultCode);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_haoxin_on_activity_result: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_haoxin_on_activity_result: enter!");
}

static void appsvr_haoxin_on_new_intent(void * ctx, void * intent) {
    appsvr_haoxin_module_t module = (appsvr_haoxin_module_t)ctx;

    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_new_intent,jobject(intent));
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_haoxin_on_new_intent: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_haoxin_on_new_intent: enter!");
}

int appsvr_haoxin_delegate_init(appsvr_haoxin_backend_t backend) {
    if (plugin_app_env_android_register_delegate(
            backend->m_module->m_app_env,
            backend->m_module,
            appsvr_haoxin_on_new_intent,
            appsvr_haoxin_on_activity_result) != 0)
    {
        CPE_ERROR(backend->m_module->m_em, "appsvr_haoxin_delegate_init: register fail!");
        return -1;
    }

    return 0;
}

void appsvr_haoxin_delegate_fini(appsvr_haoxin_backend_t backend) {
    plugin_app_env_android_unregister_delegate(backend->m_module->m_app_env, backend->m_module);
}



