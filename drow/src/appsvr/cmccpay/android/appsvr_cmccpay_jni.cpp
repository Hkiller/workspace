#include <assert.h>
#include "appsvr_cmccpay_backend.hpp"

int appsvr_cmccpay_jni_init(appsvr_cmccpay_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    /*获取Manip类 */
    jobject manip_cls = env->FindClass("com/drowgames/cmccpay/CmccPayManip");
    if (manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_cmccpay jni_init: get CmccPayManip class fail!");
        return -1;
    }
    
	backend->m_manip_cls = (jclass)env->NewGlobalRef(manip_cls);
    if (backend->m_manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_cmccpay jni_init: NewGlobalRef cmccpaySdkManip class fail!");
        return -1;
    }

    backend->m_manip_init = env->GetStaticMethodID(backend->m_manip_cls, "init", "(Landroid/app/Activity;J)V");
    assert(backend->m_manip_init);
    
    backend->m_moreGames = env->GetStaticMethodID(
        backend->m_manip_cls, "viewMoreGames", "(Landroid/app/Activity;)V");
    assert(backend->m_moreGames);

    backend->m_exitGame = env->GetStaticMethodID(
        backend->m_manip_cls, "exitGame", "(Landroid/app/Activity;)V");
    assert(backend->m_exitGame);
//     backend->m_manip_login = env->GetStaticMethodID(
//         backend->m_manip_cls, "login", "(JLandroid/app/Activity;ZZZZLjava/lang/String;ZZZZZ)V");
//     assert(backend->m_manip_login);

	backend->m_manip_dopay = env->GetStaticMethodID(
		backend->m_manip_cls,"startPayOffline", "(Landroid/app/Activity;Ljava/lang/String;)V");
	assert(backend->m_manip_dopay);
    
 	backend->m_fini = env->GetStaticMethodID(backend->m_manip_cls, "fini", "()V");
 	assert(backend->m_fini);

    /*sdk初始化 */
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_manip_init, context,backend->m_module);
    if (env->ExceptionCheck()) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_cmccpay_jni_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }
    
    return 0;
}

void appsvr_cmccpay_jni_fini(appsvr_cmccpay_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

	jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
	env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_fini);
	if (env->ExceptionCheck()) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_cmccpay_jni_fini: destroy fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		env->DeleteGlobalRef(backend->m_manip_cls);
		return;
	}

    env->DeleteGlobalRef(backend->m_manip_cls);
}

