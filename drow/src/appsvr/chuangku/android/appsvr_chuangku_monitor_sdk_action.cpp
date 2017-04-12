#include <assert.h>
#include "plugin/app_env/plugin_app_env_monitor.h"
#include "appsvr_chuangku_backend.hpp"

static int appsvr_chuangku_do_sdk_action(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    appsvr_chuangku_module_t module = (appsvr_chuangku_module_t)ctx;
    APPSVR_SDK_ACTION const * req = (APPSVR_SDK_ACTION*)req_data; 
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_call_action,
        env->NewStringUTF(req->action),
        (jint)(req->level_type),
        env->NewStringUTF(req->level_name),
        (jboolean)(req->is_success));
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call m_on_call_action fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return 0;

//     if (strcmp(req->action, "more-game") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_show_more_games_page);
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else if (strcmp(req->action, "exit") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_show_exit_page);
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else if (strcmp(req->action, "to_level") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_on_tolevel,
//             (jint)(req->level_type),
//             env->NewStringUTF(req->level_name));
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else if (strcmp(req->action, "exit_level") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_on_exitlevel,
//             (jint)(req->level_type),
//             env->NewStringUTF(req->level_name),
//             (jboolean)(req->is_success));
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else if (strcmp(req->action, "toStore") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_on_toStore);
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else if (strcmp(req->action, "exitStore") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_on_exitStore);
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else if (strcmp(req->action, "toSettlement") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_on_toSettlement);
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else if (strcmp(req->action, "exitSettlement") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_on_exitSettlement);
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else if (strcmp(req->action, "toMainView") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_on_toMainView);
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else if (strcmp(req->action, "exitMainView") == 0) {
//         env->CallStaticVoidMethod(
//             module->m_backend->m_manip_cls, module->m_backend->m_on_exitMainView);
//         if (env->ExceptionCheck()) {
//             CPE_ERROR(module->m_em, "appsvr_chuangku_do_sdk_action: call  fail!");
//             env->ExceptionDescribe();
//             env->ExceptionClear();
//             return -1;
//         }
// 
//         return 0;
//     }
//     else {
//         if (module->m_debug) {
//             CPE_INFO(module->m_em, "appsvr_chuangku_do_sdk_action: not support action = %s!", req->action);
//         }
//         return -1;
//     }
}

int appsvr_chuangku_sdk_action_monitor_init(appsvr_chuangku_backend_t backend) {
    backend->m_module->m_sdk_action_monitor = 
        plugin_app_env_monitor_create(
            backend->m_module->m_app_env,
            "appsvr_sdk_action",
            backend->m_module, appsvr_chuangku_do_sdk_action, NULL);
    if (backend->m_module->m_sdk_action_monitor == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_chuangku_plugin_init: create sdk_action fail!");
        return -1;
    }

    return 0;
}

void appsvr_chuangku_sdk_action_monitor_fini(appsvr_chuangku_backend_t backend) {
    assert(backend->m_module->m_sdk_action_monitor);
    plugin_app_env_monitor_free(backend->m_module->m_sdk_action_monitor);
    backend->m_module->m_sdk_action_monitor = NULL;
}