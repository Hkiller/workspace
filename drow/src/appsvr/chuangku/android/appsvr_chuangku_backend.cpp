#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_chuangku_backend.hpp"

static struct {
    const char * name; 
    int (*init)(appsvr_chuangku_backend_t backend);
    void (*fini)(appsvr_chuangku_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_chuangku_jni_init, appsvr_chuangku_jni_fini }
    , { "delegate", appsvr_chuangku_delegate_init, appsvr_chuangku_delegate_fini }
    , { "adk_action", appsvr_chuangku_sdk_action_monitor_init, appsvr_chuangku_sdk_action_monitor_fini }
};

int appsvr_chuangku_backend_init(appsvr_chuangku_module_t module) {
    appsvr_chuangku_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_chuangku_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_chuangku_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_backend_create: alloc fail!");
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
    
    CPE_INFO(module->m_em, "appsvr_chuangku_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_chuangku_backend_fini(appsvr_chuangku_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

int appsvr_chuangku_backend_pay_start(appsvr_chuangku_module_t module, APPSVR_PAYMENT_BUY const * req) {
    JNIEnv *env = (JNIEnv *)android_jni_env();

    char chanel_pay_id[32];
    snprintf(chanel_pay_id, sizeof(chanel_pay_id), "%d", req->chanel_pay_id);

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_manip_dopay,
        env->NewStringUTF(chanel_pay_id),
        (jint)(req->price * 100),
        env->NewStringUTF(req->product_name),
        env->NewStringUTF(req->product_desc));
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_dosdkpay: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return 0;
}

void appsvr_chuangku_on_suspend(appsvr_chuangku_module_t module) {
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_suspend);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_on_suspend: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_chuangku_on_suspend: enter!");
    return ;
}

void appsvr_chuangku_on_resume(appsvr_chuangku_module_t module) {
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_resume);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_on_resume: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_chuangku_on_resume: enter!");
    return ;
}

int appsvr_chuangku_sync_addition_attr(appsvr_chuangku_module_t module) {
    JNIEnv *env = (JNIEnv *)android_jni_env();
    jstring attr = (jstring)env->CallStaticObjectMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_get_addition_attr);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "nativeNotifyChuangkuInitResult: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    char * result = (char*)env->GetStringUTFChars(attr, NULL);
    CPE_ERROR(module->m_em, "popup_condition: enter   popup_condition=%s!",result);

    if (dr_json_read(
            module->m_addition_attr_data, dr_meta_size(module->m_addition_attr_meta),
            result,
            module->m_addition_attr_meta, module->m_em)
        < 0)
    {
        CPE_ERROR(module->m_em, "appsvr_chuangku_sync_addition_attr: set data %s fail!", result);
        env->ReleaseStringUTFChars(attr, result);
        return -1;
    }
    
    env->ReleaseStringUTFChars(attr, result);
    return 0;
}

int appsvr_chuangku_sync_attr(appsvr_chuangku_module_t module) {
    JNIEnv *env = (JNIEnv *)android_jni_env();
    jint is_supportmoregame = env->CallStaticIntMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_getMoreGameIsSupport);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_sync_attr: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    CPE_ERROR(module->m_em, "chuangku init result: enter is_supportmoregame=%d!",(int)is_supportmoregame);

    appsvr_chuangku_notify_support_more_game(module, (uint8_t)is_supportmoregame);
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_drowgames_chuangku_ChuangkuOffLineListener_nativeNotifyExitGameSupportResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result)
{
    appsvr_chuangku_module_t chuangku = (appsvr_chuangku_module_t)(ptr);
    appsvr_chuangku_notify_support_exit_game(chuangku, (uint8_t)result);
    CPE_ERROR(chuangku->m_em, "support_exit_game_result: enter   result=%d!",(int)result);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_drowgames_chuangku_ChuangkuOffLineListener_nativeNotifyPayResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result, jint service_result, jstring error_msg)
{
    appsvr_chuangku_module_t chuangku = (appsvr_chuangku_module_t)(ptr);
    char * str_error_msg = (char*)env->GetStringUTFChars(error_msg, NULL);

    APPSVR_PAYMENT_RESULT payment_result;
    bzero(&payment_result, sizeof(payment_result));
    payment_result.result = (uint8_t)result;
    payment_result.service_result = (int32_t)service_result;
    cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), str_error_msg);

    appsvr_chuangku_module_send_payment_result(chuangku, &payment_result);
    
    env->ReleaseStringUTFChars(error_msg, str_error_msg);
}