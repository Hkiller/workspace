#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr/notify/appsvr_notify_schedule.h"
#include "appsvr_notify_device_backend.hpp"

int appsvr_notify_device_backend_init(appsvr_notify_device_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_notify_device_backend_t backend;

    backend = (appsvr_notify_device_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_notify_device_backend));
    if (backend == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_notify_device_backend_init: alloc fail!");
        return -1;
    }
    
    jobject notify_device_mgr_cls = env->FindClass("com/drowgames/notify/DrowDeviceNotifyManager");
    if (notify_device_mgr_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_notify_device_backend_init: get DrowDeviceNotifyManager class fail!");
        mem_free(module->m_alloc, backend);
        return -1;
    }
    
	backend->m_notify_device_mgr_cls = (jclass)env->NewGlobalRef(notify_device_mgr_cls);
    if (backend->m_notify_device_mgr_cls == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_notify_device_backend_init: NewGlobalRef DrowDeviceNotifyManager class fail!");
        mem_free(module->m_alloc, backend);
        return -1;
    }

    backend->m_alarm_add = env->GetStaticMethodID(backend->m_notify_device_mgr_cls, "addNoticfy", 
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;III)V");
    assert(backend->m_alarm_add);
    
    backend->m_alarm_remove = env->GetStaticMethodID(backend->m_notify_device_mgr_cls, "cancelNotify", 
        "(Landroid/content/Context;I)V");
    assert(backend->m_alarm_remove);


    module->m_backend = backend;
    
    return 0;
}

void appsvr_notify_device_backend_fini(appsvr_notify_device_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_notify_device_backend_t backend = module->m_backend;
    
    assert(backend);

    assert(backend->m_notify_device_mgr_cls);
    env->DeleteGlobalRef(backend->m_notify_device_mgr_cls);
    
    mem_free(module->m_alloc, backend);
    module->m_backend = NULL;
}

int appsvr_notify_device_install_schedule(void * ctx, appsvr_notify_schedule_t schedule) {
    appsvr_notify_device_module_t module = (appsvr_notify_device_module_t)ctx;
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    const char * schedule_content = appsvr_notify_schedule_context(schedule);
    if (schedule_content == NULL) {
        schedule_content = appsvr_notify_schedule_title(schedule);
    }
    
    env->CallStaticVoidMethod(
        module->m_backend->m_notify_device_mgr_cls, module->m_backend->m_alarm_add,
        context,
        env->NewStringUTF(appsvr_notify_schedule_title(schedule)),
        env->NewStringUTF(schedule_content),
        (jint)appsvr_notify_schedule_start_time(schedule),
        (jint)appsvr_notify_schedule_id(schedule),
        (jint)appsvr_notify_schedule_repeat_time(schedule));
    
    return 0;
}

int appsvr_notify_device_update_schedule(void * ctx, appsvr_notify_schedule_t schedule) {
    appsvr_notify_device_module_t module = (appsvr_notify_device_module_t)ctx;
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);

    // env->CallStaticVoidMethod(
    //     module->m_backend->m_notify_device_mgr_cls, module->m_backend->m_alarm_add,
    //     context,
    //     env->NewStringUTF(record->title),
    //     env->NewStringUTF(record->content),
    //     (jint)record->delay_time, (jint)record->key, (jint)record->repeat_time);
    
    return 0;
}

void appsvr_notify_device_uninstall_schedule(void * ctx, appsvr_notify_schedule_t schedule) {
    appsvr_notify_device_module_t module = (appsvr_notify_device_module_t)ctx;
    JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    
    env->CallStaticVoidMethod(
        module->m_backend->m_notify_device_mgr_cls,
        module->m_backend->m_alarm_remove,
        context,
        (jint)appsvr_notify_schedule_id(schedule));
}

void appsvr_notify_device_on_suspend(appsvr_notify_device_module_t module) {
}

void appsvr_notify_device_on_resume(appsvr_notify_device_module_t module) {
}
