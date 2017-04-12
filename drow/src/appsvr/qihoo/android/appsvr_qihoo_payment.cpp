#include "appsvr_qihoo_backend.hpp"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include <stdlib.h>

static int appsvr_qihoo_payment_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req) {
	appsvr_qihoo_module_t module = *(appsvr_qihoo_module_t*)appsvr_payment_adapter_data(adapter);
	JNIEnv *env = (JNIEnv *)android_jni_env();
	jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    char trade_id[64];
    
	const char * appName="qingheliangmeng";

    const char * sep = strrchr(req->trade_id, '#');
    if (req == NULL) {
		CPE_ERROR(module->m_em, "appsvr_qihoo_dosdkpay: parse trade id %s fail!", req->trade_id);
        return -1;
    }

    const char * qihoo_user_id = sep + 1;
    cpe_str_dup_range(trade_id, sizeof(trade_id), req->trade_id, sep);
    
	env->CallStaticVoidMethod(
		module->m_backend->m_manip_cls, module->m_backend->m_manip_dopay,
		(jlong)(ptr_int_t)(module),
		context,
		(jboolean)module->m_land_space,
		env->NewStringUTF(module->m_background ? module->m_background : ""),
		env->NewStringUTF(module->m_token),
		env->NewStringUTF(qihoo_user_id),
		(jint)(req->price * 100),
		env->NewStringUTF(req->product_name),
		(jint)req->product_id,
		env->NewStringUTF(req->notify_to),
		env->NewStringUTF(appName),
		env->NewStringUTF(req->user_id),
		env->NewStringUTF(req->user_name),
		env->NewStringUTF(trade_id));
	if (env->ExceptionCheck()) {
		CPE_ERROR(module->m_em, "appsvr_qihoo_dosdkpay: call loing fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return -1;
	}

	return 0;
}

int appsvr_qihoo_payment_init(appsvr_qihoo_backend_t backend) {
	backend->m_payment_adapter = 
		appsvr_payment_adapter_create(
		backend->m_module->m_payment_module, appsvr_payment_service_360, "qihoo",
        1, 1,
		sizeof(appsvr_qihoo_module_t), appsvr_qihoo_payment_pay_start);
	if (backend->m_payment_adapter == NULL) {
		CPE_ERROR(backend->m_module->m_em, "appsvr_qihoo_login_init: create adapter fail!");
		return -1;
	}

	*(appsvr_qihoo_module_t*)appsvr_payment_adapter_data(backend->m_payment_adapter) = backend->m_module;

	return 0;
}

void appsvr_qihoo_payment_fini(appsvr_qihoo_backend_t backend) {
	if (backend->m_payment_adapter) {
		appsvr_payment_adapter_free(backend->m_payment_adapter);
		backend->m_payment_adapter = NULL;
	}
}


extern "C" {

JNIEXPORT void JNICALL Java_com_drowgames_qihoo_QihooPaymentListener_nativeNotifyResult(
    JNIEnv *env, jobject obj, jlong ptr, jint errorCode, jstring error_msg)
{
    appsvr_qihoo_module_t module = (appsvr_qihoo_module_t)(ptr);

    APPSVR_PAYMENT_RESULT payment_result;
    bzero(&payment_result, sizeof(payment_result));

    payment_result.service_result = errorCode;

    if (errorCode != 0) {
        if (errorCode == 4010201 || errorCode == 4009911) {
            payment_result.result = appsvr_payment_not_login;
        }
        else if (errorCode == -1) {
            payment_result.result = appsvr_payment_canceled;
        }
        else if (errorCode == -2) {
            payment_result.result = appsvr_payment_nunderway;
        }
        else {
            payment_result.result = appsvr_payment_failed;
        }

        char * errorMsg = (char*)env->GetStringUTFChars(error_msg, NULL);
        cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), errorMsg);
        env->ReleaseStringUTFChars(error_msg, errorMsg);
    }
    else {
        payment_result.result = appsvr_payment_success;
    }

    appsvr_payment_adapter_notify_result(module->m_backend->m_payment_adapter, &payment_result);    
}

}

