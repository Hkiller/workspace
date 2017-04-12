#include <assert.h>
#include "plugin/app_env/plugin_app_env_monitor.h"
#include "appsvr_haoxin_backend.hpp"

static int appsvr_haoxin_do_sdk_action(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    appsvr_haoxin_module_t module = (appsvr_haoxin_module_t)ctx;
    APPSVR_SDK_ACTION const * req = (APPSVR_SDK_ACTION*)req_data; 
    JNIEnv *env = (JNIEnv *)android_jni_env();

    if (strcmp(req->action, "more-game") == 0) {
        env->CallStaticVoidMethod(
            module->m_backend->m_manip_cls, module->m_backend->m_moreGames);
        if (env->ExceptionCheck()) {
            CPE_ERROR(module->m_em, "appsvr_haoxin_do_more_games: call  fail!");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return -1;

        }

        return 0;
    }
    else if (strcmp(req->action, "pause") == 0) {
        env->CallStaticVoidMethod(
            module->m_backend->m_manip_cls, module->m_backend->m_on_pause);
        if (env->ExceptionCheck()) {
            CPE_ERROR(module->m_em, "appsvr_haoxin_show_pause_page: call loing fail!");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return -1;
        }

        return 0 ;
    }
    else if (strcmp(req->action, "exit") == 0) {
        env->CallStaticVoidMethod(
            module->m_backend->m_manip_cls, module->m_backend->m_exitGame);
        if (env->ExceptionCheck()) {
            CPE_ERROR(module->m_em, "appsvr_haoxin_do_stop: call  fail!");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return -1;
        }

        return 0;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "appsvr_haoxin_do_sdk_action: not support page %s!", req->action);
        }
        return -1;
    }
}

int appsvr_haoxin_sdk_action_monitor_init(appsvr_haoxin_backend_t backend) {
    backend->m_module->m_sdk_action_monitor = 
        plugin_app_env_monitor_create(
            backend->m_module->m_app_env,
            "appsvr_sdk_action",
            backend->m_module, appsvr_haoxin_do_sdk_action, NULL);
    if (backend->m_module->m_sdk_action_monitor == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_haoxin_plugin_init: create sdk_action fail!");
        return -1;
    }

    return 0;
}

void appsvr_haoxin_sdk_action_monitor_fini(appsvr_haoxin_backend_t backend) {
    assert(backend->m_module->m_sdk_action_monitor);
    plugin_app_env_monitor_free(backend->m_module->m_sdk_action_monitor);
    backend->m_module->m_sdk_action_monitor = NULL;
}

