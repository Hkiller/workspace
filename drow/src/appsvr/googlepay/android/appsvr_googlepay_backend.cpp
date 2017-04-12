#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr_googlepay_backend.hpp"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr/payment/appsvr_payment_product.h"

static struct {
    const char * name; 
    int (*init)(appsvr_googlepay_backend_t backend);
    void (*fini)(appsvr_googlepay_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_googlepay_jni_init, appsvr_googlepay_jni_fini }
    , { "delegate", appsvr_googlepay_delegate_init, appsvr_googlepay_delegate_fini }
};

int appsvr_googlepay_backend_init(appsvr_googlepay_module_t module) {
    appsvr_googlepay_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_googlepay_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_googlepay_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_googlepay_backend_create: alloc fail!");
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
    
    CPE_INFO(module->m_em, "appsvr_googlepay_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_googlepay_backend_fini(appsvr_googlepay_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

int appsvr_googlepay_backend_pay_start(appsvr_googlepay_module_t module, APPSVR_PAYMENT_BUY const * req) {
    JNIEnv *env = (JNIEnv *)android_jni_env();
    CPE_ERROR(module->m_em, "appsvr_googlepay_backend_pay_start: enter!");
    char chanel_pay_id[32];
    snprintf(chanel_pay_id, sizeof(chanel_pay_id), "%d", req->chanel_pay_id);

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_manip_dopay,
        env->NewStringUTF(chanel_pay_id),
		(jint)(req->price * 100),
        env->NewStringUTF(req->product_name),
        env->NewStringUTF(req->product_desc));
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_googlepay_dosdkpay: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return 0;
}

int appsvr_googlepay_backend_do_sync_products(appsvr_googlepay_module_t module){
    JNIEnv *env = (JNIEnv *)android_jni_env();
    CPE_ERROR(module->m_em, "appsvr_googlepay_backend_do_sync_products: enter!");

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_do_sync_products);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_googlepay_dosdkpay: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return 0;
}

extern "C" {

JNIEXPORT void JNICALL Java_com_drowgames_googlepay_GooglepayOffLineListener_nativeNotifyPayResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result, jint service_result, jstring error_msg)
{
    appsvr_googlepay_module_t googlepay = (appsvr_googlepay_module_t)(ptr);
    char * str_error_msg = (char*)env->GetStringUTFChars(error_msg, NULL);

    APPSVR_PAYMENT_RESULT payment_result;
    bzero(&payment_result, sizeof(payment_result));
    payment_result.result = (uint8_t)result;
    payment_result.service_result = (int32_t)service_result;
    cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), str_error_msg);

    appsvr_payment_adapter_notify_result(googlepay->m_payment_adapter, &payment_result);
    
    env->ReleaseStringUTFChars(error_msg, str_error_msg);
}

JNIEXPORT void JNICALL Java_com_drowgames_googlepay_GooglepayOffLineListener_nativeNotifyProductInfoResult(
    JNIEnv *env, jobject obj, jlong ptr, jstring product_id, jstring price)
{
    appsvr_googlepay_module_t googlepay = (appsvr_googlepay_module_t)(ptr);
    char * str_product_id = (char*)env->GetStringUTFChars(product_id, NULL);
    char * str_price= (char*)env->GetStringUTFChars(price, NULL);
    if(str_product_id==NULL || str_price==NULL)
    {
        appsvr_payment_adapter_notify_product_sync_done(googlepay->m_payment_adapter);
        CPE_ERROR(googlepay->m_em, "appsvr_payment_adapter_notify_product_sync_done enter");
        return;
    }
    CPE_ERROR(googlepay->m_em, "appsvr_payment_adapter_notify_product_sync_done enter 2");
    appsvr_payment_product_create(googlepay->m_payment_module,googlepay->m_payment_adapter,str_product_id,str_price);

    env->ReleaseStringUTFChars(price, str_price);
    env->ReleaseStringUTFChars(product_id, str_product_id);
}

}

