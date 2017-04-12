#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr_damai_backend.hpp"
#include "appsvr/payment/appsvr_payment_adapter.h"

static int appsvr_damai_payment_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req) {
	appsvr_damai_module_t module = *(appsvr_damai_module_t*)appsvr_payment_adapter_data(adapter);
	JNIEnv *env = (JNIEnv *)android_jni_env();
	jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    char money[32];
    char service[32];

    snprintf(money, sizeof(money), "%d", (int)req->price);
    snprintf(service, sizeof(service), "%d", req->region_id);

	env->CallStaticVoidMethod(
		module->m_backend->m_manip_cls, module->m_backend->m_manip_dopay,
		context,
        env->NewStringUTF(req->trade_id),
		env->NewStringUTF(req->user_id),
        env->NewStringUTF(service),
		env->NewStringUTF(money),
		env->NewStringUTF(req->product_name),
        env->NewStringUTF(req->product_name));
	if (env->ExceptionCheck()) {
		CPE_ERROR(module->m_em, "appsvr_damai_dosdkpay: call loing fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return -1;
	}

	return 0;
}

int appsvr_damai_payment_init(appsvr_damai_backend_t backend) {
	backend->m_payment_adapter = 
		appsvr_payment_adapter_create(
		backend->m_module->m_payment_module,
        appsvr_payment_service_damai, "damai",
        0, 0,
		sizeof(appsvr_damai_module_t), appsvr_damai_payment_pay_start);
	if (backend->m_payment_adapter == NULL) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_damai_login_init: create adapter fail!");
		return -1;
	}

	*(appsvr_damai_module_t*)appsvr_payment_adapter_data(backend->m_payment_adapter) = backend->m_module;

	return 0;
}

void appsvr_damai_payment_fini(appsvr_damai_backend_t backend) {
	if (backend->m_payment_adapter) {
		appsvr_payment_adapter_free(backend->m_payment_adapter);
		backend->m_payment_adapter = NULL;
	}
}


extern "C" {

JNIEXPORT void JNICALL Java_com_drowgames_damai_DamaiSdkManip_nativeNotifyPaymentResult(
    JNIEnv *env, jobject obj, jlong ptr, jint errorCode, jstring error_msg)
{
    appsvr_damai_module_t module = (appsvr_damai_module_t)(ptr);

    APPSVR_PAYMENT_RESULT payment_result;
    bzero(&payment_result, sizeof(payment_result));

    payment_result.service_result = errorCode;

    if (errorCode != 0) {
        if (errorCode == 2) {
            payment_result.result = appsvr_payment_canceled;
        }
        else {
            payment_result.result = appsvr_payment_failed;

            char * errorMsg = (char*)env->GetStringUTFChars(error_msg, NULL);
            cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), errorMsg);
            env->ReleaseStringUTFChars(error_msg, errorMsg);
        }
    }
    else {
        payment_result.result = appsvr_payment_success;
    }

    appsvr_payment_adapter_notify_result(module->m_backend->m_payment_adapter, &payment_result);    
}

}

