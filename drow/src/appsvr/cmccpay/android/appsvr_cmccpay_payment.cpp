#include "appsvr_cmccpay_backend.hpp"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include <stdlib.h>

static int appsvr_cmccpay_payment_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req) {
    appsvr_cmccpay_module_t module = *(appsvr_cmccpay_module_t*)appsvr_payment_adapter_data(adapter);
    JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);

    char chanel_pay_id[64];
    snprintf(chanel_pay_id, sizeof(chanel_pay_id), "%03d", req->chanel_pay_id);

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_manip_dopay,
        context,
        env->NewStringUTF(chanel_pay_id));
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_cmccpay_dosdkpay: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return 0;
}

int appsvr_cmccpay_payment_init(appsvr_cmccpay_backend_t backend) {
	backend->m_payment_adapter = 
		appsvr_payment_adapter_create(
		backend->m_module->m_payment_module,
        appsvr_payment_service_cmccpay_offline, "cmccpay",
        0, 0,
		sizeof(appsvr_cmccpay_module_t), appsvr_cmccpay_payment_pay_start);
	if (backend->m_payment_adapter == NULL) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_cmccpay_login_init: create adapter fail!");
		return -1;
	}

	*(appsvr_cmccpay_module_t*)appsvr_payment_adapter_data(backend->m_payment_adapter) = backend->m_module;

	return 0;
}

void appsvr_cmccpay_payment_fini(appsvr_cmccpay_backend_t backend) {
	if (backend->m_payment_adapter) {
		appsvr_payment_adapter_free(backend->m_payment_adapter);
		backend->m_payment_adapter = NULL;
	}
}


extern "C" {

JNIEXPORT void JNICALL Java_com_drowgames_cmccpay_CmccOffLinePayListener_nativeNotifyResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result, jint service_result, jstring error_msg)
{
    appsvr_cmccpay_module_t cmccpay = (appsvr_cmccpay_module_t)(ptr);
    char * str_error_msg = (char*)env->GetStringUTFChars(error_msg, NULL);

    APPSVR_PAYMENT_RESULT payment_result;
    bzero(&payment_result, sizeof(payment_result));
    payment_result.result = (uint8_t)result;
    payment_result.service_result = (int32_t)service_result;
    cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), str_error_msg);

    env->ReleaseStringUTFChars(error_msg, str_error_msg);

    appsvr_payment_adapter_notify_result(cmccpay->m_backend->m_payment_adapter, &payment_result);  
}

}

