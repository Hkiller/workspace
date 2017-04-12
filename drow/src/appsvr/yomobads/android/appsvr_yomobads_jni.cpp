#include <assert.h>
#include "appsvr_yomobads_backend.hpp"

int appsvr_yomobads_jni_init(appsvr_yomobads_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    /*获取Manip类 */
    jobject manip_cls = env->FindClass("com/drowgames/yomobads/YomobadsManip");
    if (manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_yomobads jni_init: get YomobadsManip class fail!");
        return -1;
    }
    
	backend->m_manip_cls = (jclass)env->NewGlobalRef(manip_cls);
    if (backend->m_manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_yomobads jni_init: NewGlobalRef YomobadsManip class fail!");
        return -1;
    }

    backend->m_manip_init = env->GetStaticMethodID(backend->m_manip_cls, "init", "(Landroid/app/Activity;J)V");
    assert(backend->m_manip_init);

    backend->m_on_resume = env->GetStaticMethodID(
        backend->m_manip_cls, "onResume", "()V");
    assert(backend->m_on_resume);

    backend->m_on_suspend = env->GetStaticMethodID(
        backend->m_manip_cls, "onSuspend", "()V");
    assert(backend->m_on_suspend);

    backend->m_on_pause = env->GetStaticMethodID(
        backend->m_manip_cls, "onPause", "()V");
    assert(backend->m_on_pause);

	backend->m_manip_do_adsopen = env->GetStaticMethodID(
		backend->m_manip_cls,"startAdsOpen", "(Ljava/lang/String;)V");
	assert(backend->m_manip_do_adsopen);

    backend->m_on_activity_result = env->GetStaticMethodID(
        backend->m_manip_cls, "onActivityResult", "(Ljava/lang/Object;II)V");
    assert(backend->m_on_activity_result);

    backend->m_on_start = env->GetStaticMethodID(
        backend->m_manip_cls, "onStart", "()V");
    assert(backend->m_on_start);

    backend->m_on_destroy = env->GetStaticMethodID(
        backend->m_manip_cls, "onDestroy", "()V");
    assert(backend->m_on_destroy);

 	backend->m_fini = env->GetStaticMethodID(backend->m_manip_cls, "fini", "()V");
 	assert(backend->m_fini);

    /*sdk初始化 */
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_manip_init, context,backend->m_module);
    if (env->ExceptionCheck()) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_yomobads_jni_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }

    // env->CallStaticVoidMethod(
    //     module->m_backend->m_manip_cls, module->m_backend->m_on_start);
    // if (env->ExceptionCheck()) {
    //     CPE_ERROR(module->m_em, "appsvr_yomobads_on_activity_start: enter!");
    //     env->ExceptionDescribe();
    //     env->ExceptionClear();
    //     return -1;
    // }
    
    return 0;
}

void appsvr_yomobads_jni_fini(appsvr_yomobads_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

	env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_fini);
	if (env->ExceptionCheck()) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_yomobads_jni_fini: destroy fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		env->DeleteGlobalRef(backend->m_manip_cls);
		return;
	}

    // env->CallStaticVoidMethod(
    //     module->m_backend->m_manip_cls, module->m_backend->m_on_destroy);
    // if (env->ExceptionCheck()) {
    //     CPE_ERROR(module->m_em, "appsvr_yomobads_on_activity_destroy: call fail!");
    //     env->ExceptionDescribe();
    //     env->ExceptionClear();
    //     return;
    // }

    env->DeleteGlobalRef(backend->m_manip_cls);
}
