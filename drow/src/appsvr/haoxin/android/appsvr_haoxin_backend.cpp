#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_haoxin_backend.hpp"

static struct {
    const char * name; 
    int (*init)(appsvr_haoxin_backend_t backend);
    void (*fini)(appsvr_haoxin_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_haoxin_jni_init, appsvr_haoxin_jni_fini }
    , { "delegate", appsvr_haoxin_delegate_init, appsvr_haoxin_delegate_fini }
    , { "sdk-action", appsvr_haoxin_sdk_action_monitor_init, appsvr_haoxin_sdk_action_monitor_fini }
};

int appsvr_haoxin_backend_init(appsvr_haoxin_module_t module) {
    appsvr_haoxin_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_haoxin_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_haoxin_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_haoxin_backend_create: alloc fail!");
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
    
    CPE_INFO(module->m_em, "appsvr_haoxin_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_haoxin_backend_fini(appsvr_haoxin_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

void appsvr_haoxin_on_suspend(appsvr_haoxin_module_t module) {
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_suspend);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_haoxin_on_suspend: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_haoxin_on_suspend: enter!");
    return ;
}

void appsvr_haoxin_on_resume(appsvr_haoxin_module_t module) {
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_resume);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_haoxin_on_resume: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_haoxin_on_resume: enter!");
    return ;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_drowgames_haoxin_HaoxinOffLineListener_nativeNotifyExitCancelResultw(
    JNIEnv *env, jobject obj, jlong ptr)
{
}


extern "C"
JNIEXPORT void JNICALL
Java_com_drowgames_haoxin_HaoxinOffLineListener_nativeNotifyMoreGameSupportResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result)
{
    appsvr_haoxin_module_t haoxin = (appsvr_haoxin_module_t)(ptr);
    appsvr_haoxin_notify_support_more_game(haoxin, (uint8_t)result);  
    CPE_ERROR(haoxin->m_em, "support_More_game_result: enter   result=%d!",result);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_drowgames_haoxin_HaoxinOffLineListener_nativeNotifyExitGameSupportResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result)
{
    appsvr_haoxin_module_t haoxin = (appsvr_haoxin_module_t)(ptr);
    appsvr_haoxin_notify_support_exit_game(haoxin, (uint8_t)result);  
    CPE_ERROR(haoxin->m_em, "support_exit_game_result: enter   result=%d!",(int)result);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_drowgames_haoxin_HaoxinOffLineListener_nativeNotifyGetPayScreenResult(
    JNIEnv *env, jobject obj, jlong ptr, jstring payscreen)
{
    appsvr_haoxin_module_t haoxin = (appsvr_haoxin_module_t)(ptr);

    char * mode = (char*)env->GetStringUTFChars(payscreen, NULL);
    char buf[64];
    cpe_str_dup(buf, sizeof(buf), mode);
    
    appsvr_haoxin_notify_payscreen(haoxin,buf);
    CPE_ERROR(haoxin->m_em, "payscreen_result: enter   payscreen=%s!",buf);

    env->ReleaseStringUTFChars(payscreen, mode);
}

int appsvr_haoxin_backend_pay_start(appsvr_haoxin_module_t module, APPSVR_PAYMENT_BUY const * req) {
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
        CPE_ERROR(module->m_em, "appsvr_haoxin_dosdkpay: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_drowgames_haoxin_HaoxinOffLineListener_nativeNotifyPayResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result, jint service_result, jstring error_msg)
{
    appsvr_haoxin_module_t haoxin = (appsvr_haoxin_module_t)(ptr);
    char * str_error_msg = (char*)env->GetStringUTFChars(error_msg, NULL);

    APPSVR_PAYMENT_RESULT payment_result;
    bzero(&payment_result, sizeof(payment_result));
    payment_result.result = (uint8_t)result;
    payment_result.service_result = (int32_t)service_result;
    cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), str_error_msg);

    appsvr_haoxin_module_send_payment_result(haoxin, &payment_result);
    
    env->ReleaseStringUTFChars(error_msg, str_error_msg);
}

