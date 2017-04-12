#include <assert.h>
#include "appsvr_qihoo_backend.hpp"

int appsvr_qihoo_jni_init(appsvr_qihoo_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    /*获取Manip类 */
    jobject manip_cls = env->FindClass("com/drowgames/qihoo/QihooSdkManip");
    if (manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_qihoo jni_init: get QihooSdkManip class fail!");
        return -1;
    }
    
	backend->m_manip_cls = (jclass)env->NewGlobalRef(manip_cls);
    if (backend->m_manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_qihoo jni_init: NewGlobalRef QihooSdkManip class fail!");
        return -1;
    }

    backend->m_manip_init = env->GetStaticMethodID(backend->m_manip_cls, "init", "(Landroid/app/Activity;)V");
    assert(backend->m_manip_init);
    
    backend->m_manip_login = env->GetStaticMethodID(
        backend->m_manip_cls, "login", "(JLandroid/app/Activity;ZZZZLjava/lang/String;ZZZZZ)V");
    assert(backend->m_manip_login);

	backend->m_manip_dopay = env->GetStaticMethodID(
		backend->m_manip_cls,"doSdkPay", "(JLandroid/app/Activity;ZLjava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	assert(backend->m_manip_dopay);
    
	backend->m_manip_destroy = env->GetStaticMethodID(backend->m_manip_cls, "onDestroy", "(Landroid/app/Activity;)V");
	assert(backend->m_manip_destroy);

    /*sdk初始化 */
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_manip_init, context);
    if (env->ExceptionCheck()) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_qihoo_jni_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }
    
    return 0;
}

void appsvr_qihoo_jni_fini(appsvr_qihoo_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

	jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
	env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_manip_destroy, context);
	if (env->ExceptionCheck()) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_qihoo_jni_fini: destroy fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		env->DeleteGlobalRef(backend->m_manip_cls);
		return;
	}

    env->DeleteGlobalRef(backend->m_manip_cls);
}

