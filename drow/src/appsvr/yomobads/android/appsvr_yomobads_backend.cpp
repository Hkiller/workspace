#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "appsvr_yomobads_backend.hpp"
#include "../appsvr_yomobads_module_i.h"
#include "appsvr/ad/appsvr_ad_adapter.h"
#include "appsvr/ad/appsvr_ad_module.h"
#include "appsvr/ad/appsvr_ad_request.h"
#include "appsvr/ad/appsvr_ad_action.h"
static struct {
    const char * name; 
    int (*init)(appsvr_yomobads_backend_t backend);
    void (*fini)(appsvr_yomobads_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_yomobads_jni_init, appsvr_yomobads_jni_fini }
};

int appsvr_yomobads_backend_init(appsvr_yomobads_module_t module) {
    appsvr_yomobads_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_yomobads_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_yomobads_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_yomobads_backend_create: alloc fail!");
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
    
    CPE_INFO(module->m_em, "appsvr_yomobads_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_yomobads_backend_fini(appsvr_yomobads_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

int appsvr_yomobads_backend_open_start(appsvr_yomobads_module_t module, const char* sceneID){
    JNIEnv *env = (JNIEnv *)android_jni_env();
    CPE_ERROR(module->m_em, "appsvr_yomobads_backend_open_start: enter!");
    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_manip_do_adsopen,
        env->NewStringUTF(sceneID));

    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_yomobads_dosdkpay: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return 0;
 }

void appsvr_yomobads_on_suspend(appsvr_yomobads_module_t module) {
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_suspend);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_yomobads_on_suspend: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_yomobads_on_suspend: enter!");
    return ;
}

void appsvr_yomobads_on_resume(appsvr_yomobads_module_t module) {
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_resume);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_yomobads_on_resume: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_yomobads_on_resume: enter!");
    return ;
}

void appsvr_yomobads_on_pause(appsvr_yomobads_module_t module, uint8_t is_pause) {
    if(is_pause)
    {
        JNIEnv *env = (JNIEnv *)android_jni_env();

        env->CallStaticVoidMethod(
            module->m_backend->m_manip_cls, module->m_backend->m_on_pause);
        if (env->ExceptionCheck()) {
            CPE_ERROR(module->m_em, "appsvr_yomobads_on_pause: call loing fail!");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }
        CPE_ERROR(module->m_em, "appsvr_yomobads_on_pause: enter!");
        return ;
    }
}

int appsvr_yomobads_read_adaction_data(appsvr_yomobads_module_t module,cfg_t cfg){
    struct cfg_it scene_it;
    cfg_t scene_cfg;
    cfg_it_init(&scene_it, cfg_find_cfg(cfg, "android.seans"));
    while((scene_cfg = cfg_it_next(&scene_it))) {
        struct cfg_it bind_to_it;
        const char * scene_id;
        cfg_t bind_to_cfg;
        size_t id_len;

        scene_id  = cfg_get_string(scene_cfg, "id", NULL);
        if (scene_id == NULL) {
            APP_CTX_ERROR(module->m_app, "appsvr_yomobads_module_app_init:scene id not configured!");
            return -1;
        }

        id_len = strlen(scene_id) + 1;

        cfg_it_init(&bind_to_it, cfg_find_cfg(scene_cfg, "bind-to"));
        while((bind_to_cfg = cfg_it_next(&bind_to_it))) {
            const char * action_name;
            appsvr_ad_action_t action;

            action_name  = cfg_as_string(bind_to_cfg, NULL);
            if (action_name == NULL) {
                APP_CTX_ERROR(module->m_app, "appsvr_yomobads_module_app_init:bind-to format error!");
                return -1;
            }

            action = appsvr_ad_action_create(module->m_ad_adapter, action_name, id_len);
            if (action == NULL) {
                APP_CTX_ERROR(module->m_app, "appsvr_yomobads_module_app_init:action create fail!!", action_name);
                return -1;
            }

            memcpy(appsvr_ad_action_data(action), scene_id, id_len);
        }
    }

    return 0;
}

extern "C" {

JNIEXPORT void JNICALL Java_com_drowgames_yomobads_YomobadsListener_nativeNotifyAdResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result)
{
    appsvr_yomobads_module_t yomobads = (appsvr_yomobads_module_t)(ptr);
    appsvr_ad_request_t request = appsvr_ad_request_find_by_id(yomobads->m_ad_module,yomobads->m_request_id);
    appsvr_ad_request_set_result(request,(appsvr_ad_result_t)result);
}

}
