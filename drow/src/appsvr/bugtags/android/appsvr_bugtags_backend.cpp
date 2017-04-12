#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr_bugtags_backend.hpp"

int appsvr_bugtags_backend_init(appsvr_bugtags_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_bugtags_backend_t backend;
    cfg_t bugtags_cfg = cfg_find_cfg(gd_app_cfg(module->m_app), "args.bugtags");
    cfg_t ios_cfg = cfg_find_cfg(bugtags_cfg, "ios");

    const char * runing_mode = cfg_get_string(bugtags_cfg, "runing-mode", NULL);
    if (runing_mode == NULL) {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: runing-mode not configured");
        return -1;
    }

    jint mode_e;
    if (strcmp(runing_mode, "silent") == 0) {
        mode_e = 0;
    }
    else if (strcmp(runing_mode, "bubble") == 0) {
        mode_e = 2;
    }
    else if (strcmp(runing_mode, "shake") == 0) {
        mode_e = 1;
    }
    else {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: runing-mode %s unknown, shoud be (silent/bubble/shake)", runing_mode);
        return -1;
    }
    
    const char * app_key = cfg_get_string(ios_cfg, "app-key", NULL);
    if (app_key == NULL) {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: app-key not configured");
        return -1;
    }

    backend = (appsvr_bugtags_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_bugtags_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: alloc fail!");
        return -1;
    }
    backend->m_app_env = plugin_app_env_module_find_nc(module->m_app, NULL);
    assert(backend->m_app_env);

    jobject bugtags_cls = env->FindClass("com/drowgames/bugtags/BugtagsManip");
    if (bugtags_cls == NULL) {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: get BugtagsManip class fail!");
        mem_free(module->m_alloc, backend);
        return -1;
    }
    
	backend->m_bugtags_cls = (jclass)env->NewGlobalRef(bugtags_cls);
    if (backend->m_bugtags_cls == NULL) {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: NewGlobalRef IAppPayUtils class fail!");
        mem_free(module->m_alloc, backend);
        return -1;
    }

    backend->m_init = env->GetStaticMethodID(
        backend->m_bugtags_cls, "init", "(Landroid/app/Activity;Ljava/lang/String;I)V");
    assert(backend->m_init);

    backend->m_on_pause = env->GetStaticMethodID(backend->m_bugtags_cls, "onPause", "(Landroid/app/Activity;)V");
    assert(backend->m_on_pause);

    backend->m_on_resume = env->GetStaticMethodID(backend->m_bugtags_cls, "onResume", "(Landroid/app/Activity;)V");
    assert(backend->m_on_resume);
    
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_app_env);
    env->CallStaticVoidMethod(
        backend->m_bugtags_cls, backend->m_init,
        context, env->NewStringUTF(app_key), mode_e);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_bugtags_backend_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        
        env->DeleteGlobalRef(backend->m_bugtags_cls);
        mem_free(module->m_alloc, backend);
        return -1;
    }
    
    module->m_backend = backend;
    return 0;
}

void appsvr_bugtags_backend_fini(appsvr_bugtags_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_bugtags_backend_t backend = module->m_backend;
    
    assert(backend);

    assert(backend->m_bugtags_cls);
    
    env->DeleteGlobalRef(backend->m_bugtags_cls);
    
    mem_free(module->m_alloc, backend);
    module->m_backend = NULL;
}

