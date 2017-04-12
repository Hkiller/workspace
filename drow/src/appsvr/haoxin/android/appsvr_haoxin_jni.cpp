#include <assert.h>
#include "appsvr_haoxin_backend.hpp"

int appsvr_haoxin_jni_init(appsvr_haoxin_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    /*获取Manip类 */
    jobject manip_cls = env->FindClass("com/drowgames/haoxin/HaoxinPayManip");
    if (manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_haoxin jni_init: get HaoxinPayManip class fail!");
        return -1;
    }
    
	backend->m_manip_cls = (jclass)env->NewGlobalRef(manip_cls);
    if (backend->m_manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_haoxin jni_init: NewGlobalRef HaoxinPayManip class fail!");
        return -1;
    }

    backend->m_manip_init = env->GetStaticMethodID(backend->m_manip_cls, "init", "(Landroid/app/Activity;J)V");
    assert(backend->m_manip_init);
    
    backend->m_moreGames = env->GetStaticMethodID(
        backend->m_manip_cls, "viewMoreGames", "()V");
    assert(backend->m_moreGames);

    backend->m_exitGame = env->GetStaticMethodID(
        backend->m_manip_cls, "exitGame", "()V");
    assert(backend->m_exitGame);

    backend->m_sdkExitGameSupport = env->GetStaticMethodID(
        backend->m_manip_cls, "isSupportExitGame", "()V");
    assert(backend->m_sdkExitGameSupport);

    backend->m_sdkMoreGameSupport = env->GetStaticMethodID(
        backend->m_manip_cls, "isSupportMoreGame", "()V");
    assert(backend->m_sdkMoreGameSupport);

    backend->m_on_resume = env->GetStaticMethodID(
        backend->m_manip_cls, "onResume", "()V");
    assert(backend->m_on_resume);

    backend->m_on_suspend = env->GetStaticMethodID(
        backend->m_manip_cls, "onSuspend", "()V");
    assert(backend->m_on_suspend);

    backend->m_on_pause = env->GetStaticMethodID(
        backend->m_manip_cls, "pause_game", "()V");
    assert(backend->m_on_pause);

	backend->m_manip_dopay = env->GetStaticMethodID(
		backend->m_manip_cls,"startPayOffline", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)V");
	assert(backend->m_manip_dopay);

    backend->m_on_activity_result = env->GetStaticMethodID(
        backend->m_manip_cls, "onActivityResult", "(Ljava/lang/Object;II)V");
    assert(backend->m_on_activity_result);

    backend->m_on_new_intent = env->GetStaticMethodID(
        backend->m_manip_cls, "onNewIntent", "(Ljava/lang/Object;)V");
    assert(backend->m_on_new_intent);

 	backend->m_fini = env->GetStaticMethodID(backend->m_manip_cls, "fini", "()V");
 	assert(backend->m_fini);

    /*sdk初始化 */
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_manip_init, context,backend->m_module);
    if (env->ExceptionCheck()) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_haoxin_jni_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }
    
    return 0;
}

void appsvr_haoxin_jni_fini(appsvr_haoxin_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

	env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_fini);
	if (env->ExceptionCheck()) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_haoxin_jni_fini: destroy fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		env->DeleteGlobalRef(backend->m_manip_cls);
		return;
	}

    env->DeleteGlobalRef(backend->m_manip_cls);
}

