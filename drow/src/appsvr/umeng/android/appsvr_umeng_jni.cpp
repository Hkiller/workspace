#include <cassert>
#include "appsvr_umeng_backend.hpp"

#define LOAD_MANIP_METHOD(__id, __static, __name, __signer) \
    appsvr_umeng_jni_load_method( \
        backend, env, backend-> __id, __static, backend->m_manip_cls, __name, __signer)

#define LOAD_AGENT_METHOD(__id, __static, __name, __signer) \
    appsvr_umeng_jni_load_method( \
        backend, env, backend-> __id, __static, backend->m_agent_cls, __name, __signer)

int appsvr_umeng_jni_init(appsvr_umeng_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    jobject manip_cls = env->FindClass("com/drowgames/umeng/UmengManip");
    if (manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_umeng_jni_init: get UmengManip class fail!");
        return -1;
    }
	backend->m_manip_cls = (jclass)env->NewGlobalRef(manip_cls);
    if (backend->m_manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_umeng_jni_init: NewGlobalRef UmengManip class fail!");
        return -1;
    }

    jobject agent_cls = env->FindClass("com/umeng/analytics/game/UMGameAgent");
    if (agent_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_umeng_jni_init: get UMGameAgent class fail!");
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }
	backend->m_agent_cls = (jclass)env->NewGlobalRef(agent_cls);
    if (backend->m_agent_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_umeng_jni_init: NewGlobalRef UMGameAgent class fail!");
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }

    backend->m_map_cls = (jclass)env->FindClass("java/util/HashMap");
    if (backend->m_map_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_umeng_jni_init: get HashMap class fail!");
        env->DeleteGlobalRef(backend->m_agent_cls);
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }
    backend->m_map_cls = (jclass)env->NewGlobalRef(backend->m_map_cls);
    assert(backend->m_map_cls);
    backend->m_map_init = env->GetMethodID(backend->m_map_cls, "<init>", "()V");
    assert(backend->m_map_init);
    backend->m_map_put = env->GetMethodID(backend->m_map_cls, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    assert(backend->m_map_put);

    if (LOAD_MANIP_METHOD(m_manip_init, true, "init", "(Landroid/content/Context;ZLjava/lang/String;Ljava/lang/String;)V") != 0
        || LOAD_AGENT_METHOD(m_agent_report_error, true, "reportError", "(Landroid/content/Context;Ljava/lang/String;)V") != 0        
        || LOAD_AGENT_METHOD(m_agent_on_pause, true, "onPause", "(Landroid/content/Context;)V") != 0
        || LOAD_AGENT_METHOD(m_agent_on_resume, true, "onResume", "(Landroid/content/Context;)V") != 0
        || LOAD_AGENT_METHOD(m_agent_on_kill, true, "onKillProcess", "(Landroid/content/Context;)V") != 0
        || LOAD_AGENT_METHOD(m_agent_set_debug, true, "setDebugMode", "(Z)V") != 0
        || LOAD_AGENT_METHOD(m_agent_set_page_track, true, "openActivityDurationTrack", "(Z)V") != 0
        || LOAD_AGENT_METHOD(m_agent_on_page_start, true, "onPageStart", "(Ljava/lang/String;)V") != 0
        || LOAD_AGENT_METHOD(m_agent_on_page_end, true, "onPageEnd", "(Ljava/lang/String;)V") != 0
        || LOAD_AGENT_METHOD(m_agent_on_event, true, "onEvent", "(Landroid/content/Context;Ljava/lang/String;Ljava/util/Map;)V") != 0
        )
    {
        env->DeleteGlobalRef(backend->m_agent_cls);
        backend->m_agent_cls = NULL;
        env->DeleteGlobalRef(backend->m_map_cls);
        backend->m_map_cls = NULL;
        env->DeleteGlobalRef(backend->m_manip_cls);
        backend->m_manip_cls = NULL;
        return -1;
    }
    
    return 0;
}

void appsvr_umeng_jni_fini(appsvr_umeng_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    env->DeleteGlobalRef(backend->m_manip_cls);
    backend->m_manip_cls = NULL;
    
	assert(backend->m_agent_cls);
    
    env->DeleteGlobalRef(backend->m_agent_cls);
    backend->m_agent_cls = NULL;

    env->DeleteGlobalRef(backend->m_map_cls);
    backend->m_map_cls = NULL;
}

int appsvr_umeng_jni_load_method(
    appsvr_umeng_backend_t backend, JNIEnv* env, jmethodID & id,
    bool isstatic, const jclass claxx, const char* methodname, const char* methodsigner)
{
	id = isstatic ? env->GetStaticMethodID(claxx, methodname, methodsigner) : env->GetMethodID(claxx, methodname, methodsigner);
    if (id == NULL) {
        CPE_ERROR(
            backend->m_module->m_em, "appsvr_umeng_jni_load_method: get %sfunction %s %s fail!",
            isstatic ? "static " : "",
            methodname, methodsigner);
        return -1;
    }
    
    return 0;
}
