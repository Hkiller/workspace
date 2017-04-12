#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr_cmccpay_backend.hpp"

static struct {
    const char * name; 
    int (*init)(appsvr_cmccpay_backend_t backend);
    void (*fini)(appsvr_cmccpay_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_cmccpay_jni_init, appsvr_cmccpay_jni_fini }
    , { "payment", appsvr_cmccpay_payment_init, appsvr_cmccpay_payment_fini }
};

int appsvr_cmccpay_backend_init(appsvr_cmccpay_module_t module) {
    appsvr_cmccpay_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_cmccpay_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_cmccpay_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_cmccpay_backend_create: alloc fail!");
        return -1;
    }

    bzero(backend, sizeof(*backend));
    backend->m_module = module;

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(backend) != 0) {
            CPE_ERROR(module->m_em, "appsvr_umeng_backend_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(backend);
            }

            mem_free(module->m_alloc, backend);
            return -1;
        }
    }
    
    CPE_INFO(module->m_em, "appsvr_cmccpay_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_cmccpay_backend_fini(appsvr_cmccpay_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

int appsvr_cmccpay_show_stop_page(appsvr_cmccpay_module_t module) {
    //APP_ENV_STOP const * req = req_data; 
    JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_exitGame,
        context);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_cmccpay_do_stop: call  fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return 0;
}

int appsvr_cmccpay_show_more_game_page(appsvr_cmccpay_module_t module) {
    //CLI_APP_ENV_MORE_GAMES const * req = req_data; 
    JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_moreGames,
        context);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_cmccpay_do_more_games: call  fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
            
    }

    return 1;
}


extern "C" {

    JNIEXPORT void JNICALL Java_com_drowgames_cmccpay_CmccOffLineExitGameListener_nativeNotifyResult(
        JNIEnv *env, jobject obj, jlong ptr, jint result)
    {
        appsvr_cmccpay_module_t cmccpay = (appsvr_cmccpay_module_t)(ptr);
    }

}
