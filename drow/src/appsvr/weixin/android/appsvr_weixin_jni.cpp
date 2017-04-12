#include <assert.h>
#include "appsvr_weixin_backend.hpp"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"

int appsvr_weixin_jni_init(appsvr_weixin_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    /*获取Manip类 */
    jobject manip_cls = env->FindClass("com/drowgames/weixin/WeixinManip");
    if (manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_weixin jni_init: get WeixinManip class fail!");
        return -1;
    }
    
	backend->m_manip_cls = (jclass)env->NewGlobalRef(manip_cls);
    if (backend->m_manip_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_weixin jni_init: NewGlobalRef WeixinManip class fail!");
        return -1;
    }

    backend->m_manip_init = env->GetStaticMethodID(backend->m_manip_cls, "init", "(JLandroid/app/Activity;Ljava/lang/String;)V");
    assert(backend->m_manip_init);
    
	backend->m_manip_fini = env->GetStaticMethodID(backend->m_manip_cls, "fini", "()V");
	assert(backend->m_manip_fini);

    backend->m_manip_login = env->GetStaticMethodID(backend->m_manip_cls, "login", "(Ljava/lang/String;Ljava/lang/String;)V");
    assert(backend->m_manip_login);
    
    // backend->m_manip_login = env->GetStaticMethodID(
    //     backend->m_manip_cls, "login", "(JLandroid/app/Activity;ZZZZLjava/lang/String;ZZZZZ)V");
    // assert(backend->m_manip_login);

    /*sdk初始化 */
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_module->m_app_env);
    env->CallStaticVoidMethod(
        backend->m_manip_cls, backend->m_manip_init, 
        (jlong)(ptr_int_t)backend->m_module,
        context,
        env->NewStringUTF(backend->m_module->m_appid));
    if (env->ExceptionCheck()) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_weixin_jni_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteGlobalRef(backend->m_manip_cls);
        return -1;
    }
    
    return 0;
}

void appsvr_weixin_jni_fini(appsvr_weixin_backend_t backend) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

	env->CallStaticVoidMethod(backend->m_manip_cls, backend->m_manip_fini);
	if (env->ExceptionCheck()) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_weixin_jni_fini: destroy fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

    env->DeleteGlobalRef(backend->m_manip_cls);
}

