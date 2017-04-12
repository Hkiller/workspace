#include <cassert>
#include "appsvr_umeng_backend.hpp"

int appsvr_umeng_agent_init(appsvr_umeng_backend_t backend) {
    appsvr_umeng_module_t module = backend->m_module;
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    assert(context);

    env->CallStaticVoidMethod(backend->m_agent_cls, backend->m_agent_set_debug, (jboolean)module->m_debug);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_umeng_agent_init: call setDebugMode fail!");
        return -1;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "appsvr_umeng_agent_init: call setDebugMode success!");
        }
    }

    env->CallStaticVoidMethod(backend->m_agent_cls, backend->m_agent_set_page_track, (jboolean)false);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_umeng_agent_init: call openActivityDurationTrack fail!");
        return -1;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "appsvr_umeng_agent_init: call openActivityDurationTrack success!");
        }
    }
    
	env->CallStaticVoidMethod(
        backend->m_manip_cls, backend->m_manip_init,
        context,
#if DEBUG
        true,
#else
        false,
#endif
        env->NewStringUTF(module->m_app_key),
        env->NewStringUTF(module->m_chanel));
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_umeng_agent_init: call init fail!");
        return -1;
    }
    else if (module->m_debug) {
        CPE_INFO(module->m_em, "appsvr_umeng_agent_init: call init success!");
    }
    
    return 0;
}

void appsvr_umeng_agent_fini(appsvr_umeng_backend_t backend) {
    appsvr_umeng_module_t module = backend->m_module;
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    assert(context);

    env->CallStaticVoidMethod(backend->m_agent_cls, backend->m_agent_on_kill, context);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_umeng_agent_fini: call onKillProcess fail!");
    }
    else if (module->m_debug) {
        CPE_INFO(module->m_em, "appsvr_umeng_agent_fini: call onKillProcess success!");
    }
}

