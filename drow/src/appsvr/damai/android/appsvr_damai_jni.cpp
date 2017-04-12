#include <assert.h>
#include "appsvr_damai_backend.hpp"

int appsvr_damai_jni_init(appsvr_damai_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    /*获取Manip类 */
    jobject manip_cls = env->FindClass("com/drowgames/damai/DamaiSdkManip");
    if (manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_damai jni_init: get DamaiSdkManip class fail!");
        return -1;
    }
    
	backend->m_manip_cls = (jclass)env->NewGlobalRef(manip_cls);
    if (backend->m_manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_damai jni_init: NewGlobalRef DamaiSdkManip class fail!");
        return -1;
    }

    backend->m_manip_init = env->GetStaticMethodID(backend->m_manip_cls, "init", "(Landroid/app/Activity;J)V");
    assert(backend->m_manip_init);

	backend->m_manip_fini = env->GetStaticMethodID(backend->m_manip_cls, "fini", "()V");
	assert(backend->m_manip_fini);

	backend->m_manip_suspend = env->GetStaticMethodID(backend->m_manip_cls, "suspend", "()V");
	assert(backend->m_manip_suspend);

	backend->m_manip_resume = env->GetStaticMethodID(backend->m_manip_cls, "resume", "()V");
	assert(backend->m_manip_resume);
    
    backend->m_manip_login = env->GetStaticMethodID(
        backend->m_manip_cls, "login", "(Landroid/app/Activity;)V");
    assert(backend->m_manip_login);

	backend->m_manip_dopay = env->GetStaticMethodID(
		backend->m_manip_cls,"doSdkPay", "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    assert(backend->m_manip_dopay);

	backend->m_manip_set_userinfo = env->GetStaticMethodID(backend->m_manip_cls,"setUserInfo", "(Ljava/lang/String;)V");
    assert(backend->m_manip_set_userinfo);
    
    /*sdk初始化 */
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_manip_init, context, (jlong)backend->m_module);
    if (env->ExceptionCheck()) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_damai_jni_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }
    
    return 0;
}

void appsvr_damai_jni_fini(appsvr_damai_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

	env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_manip_fini);
	if (env->ExceptionCheck()) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_damai_jni_fini: fini fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		env->DeleteGlobalRef(backend->m_manip_cls);
		return;
	}

    env->DeleteGlobalRef(backend->m_manip_cls);
}

