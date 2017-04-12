#include <assert.h>
#include "appsvr_facebook_share_backend.hpp"

int appsvr_facebook_share_jni_init(appsvr_facebook_share_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    /*获取Manip类 */
    jobject manip_cls = env->FindClass("com/drowgames/facebook_share/FacebookShareManip");
    if (manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_facebook_share jni_init: get FacebookShareManip class fail!");
        return -1;
    }
    
	backend->m_manip_cls = (jclass)env->NewGlobalRef(manip_cls);
    if (backend->m_manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_facebook_share jni_init: NewGlobalRef FacebookShareManip class fail!");
        return -1;
    }

    backend->m_init = env->GetStaticMethodID(backend->m_manip_cls, "init", "(Landroid/app/Activity;J)V");
    assert(backend->m_init);

	backend->m_onShare = env->GetStaticMethodID(
		backend->m_manip_cls,"onShare", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	assert(backend->m_onShare);

    backend->m_on_activity_result = env->GetStaticMethodID(
        backend->m_manip_cls, "onActivityResult", "(Ljava/lang/Object;II)V");
    assert(backend->m_on_activity_result);

 	backend->m_fini = env->GetStaticMethodID(backend->m_manip_cls, "fini", "()V");
 	assert(backend->m_fini);

    /*sdk初始化 */
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_init, context,backend->m_module);
    if (env->ExceptionCheck()) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_facebook_share_jni_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteGlobalRef(backend->m_manip_cls);
        //return -1;
    }
    
    return 0;
}

void appsvr_facebook_share_jni_fini(appsvr_facebook_share_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

	env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_fini);
	if (env->ExceptionCheck()) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_facebook_share_jni_fini: destroy fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		env->DeleteGlobalRef(backend->m_manip_cls);
		return;
	}

    env->DeleteGlobalRef(backend->m_manip_cls);
}