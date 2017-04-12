#include <assert.h>
#include "appsvr_chuangku_backend.hpp"

int appsvr_chuangku_jni_init(appsvr_chuangku_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    /*获取Manip类 */
    jobject manip_cls = env->FindClass("com/drowgames/chuangku/ChuangkuPayManip");
    if (manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_chuangku jni_init: get ChuangkuPayManip class fail!");
        return -1;
    }
    
	backend->m_manip_cls = (jclass)env->NewGlobalRef(manip_cls);
    if (backend->m_manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_chuangku jni_init: NewGlobalRef chuangkuPayManip class fail!");
        return -1;
    }

    backend->m_manip_init = env->GetStaticMethodID(backend->m_manip_cls, "init", "(Landroid/app/Activity;J)V");
    assert(backend->m_manip_init);
    
    backend->m_fini = env->GetStaticMethodID(backend->m_manip_cls, "fini", "()V");
    assert(backend->m_fini);

    backend->m_manip_dopay = env->GetStaticMethodID(
        backend->m_manip_cls,"startPay", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)V");
    assert(backend->m_manip_dopay);

    backend->m_getMoreGameIsSupport = env->GetStaticMethodID(
        backend->m_manip_cls, "getIsMoreGames", "()I");
    assert(backend->m_getMoreGameIsSupport);

//     backend->m_show_more_games_page = env->GetStaticMethodID(
//         backend->m_manip_cls, "onShowMoreGamesPage", "()V");
//     assert(backend->m_show_more_games_page);
// 
//     backend->m_show_exit_page = env->GetStaticMethodID(
//         backend->m_manip_cls, "onShowExitGamePage", "()V");
//     assert(backend->m_show_exit_page);

    backend->m_on_get_addition_attr = env->GetStaticMethodID(
        backend->m_manip_cls,"getAdditionAttr", "()Ljava/lang/String;");
    assert(backend->m_on_get_addition_attr);

    backend->m_on_resume = env->GetStaticMethodID(
        backend->m_manip_cls, "onResume", "()V");
    assert(backend->m_on_resume);

    backend->m_on_suspend = env->GetStaticMethodID(
        backend->m_manip_cls, "onSuspend", "()V");
    assert(backend->m_on_suspend);

    backend->m_on_activity_result = env->GetStaticMethodID(
        backend->m_manip_cls, "onActivityResult", "(Ljava/lang/Object;II)V");
    assert(backend->m_on_activity_result);

    backend->m_on_new_intent = env->GetStaticMethodID(
        backend->m_manip_cls, "onNewIntent", "(Ljava/lang/Object;)V");
    assert(backend->m_on_new_intent);

    backend->m_on_call_action = env->GetStaticMethodID(
        backend->m_manip_cls, "onCallAction", "(Ljava/lang/String;ILjava/lang/String;Z)V");
    assert(backend->m_on_call_action);

    /*sdk初始化 */
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_manip_init, context,backend->m_module);
    if (env->ExceptionCheck()) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_chuangku_jni_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }
    
    return 0;
}

void appsvr_chuangku_jni_fini(appsvr_chuangku_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

	env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_fini);
	if (env->ExceptionCheck()) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_chuangku_jni_fini: destroy fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		env->DeleteGlobalRef(backend->m_manip_cls);
		return;
	}

    env->DeleteGlobalRef(backend->m_manip_cls);
}