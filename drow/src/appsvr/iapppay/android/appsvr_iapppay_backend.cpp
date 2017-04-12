#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_iapppay_backend.hpp"
#include "../appsvr_iapppay_payment_adapter_i.h"

int appsvr_iapppay_backend_init(appsvr_iapppay_module_t iapppay) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_iapppay_backend_t backend;

    cfg_t global_cfg = cfg_find_cfg(gd_app_cfg(iapppay->m_app), "args");
    cfg_t iapppay_cfg = cfg_find_cfg(global_cfg, "iapppay");
    cfg_t android_cfg = cfg_find_cfg(iapppay_cfg, "android");

    if (appsvr_iapppay_module_set_chanel(iapppay, cfg_get_string(global_cfg, "chanel", NULL)) != 0
        || appsvr_iapppay_module_set_url(iapppay, cfg_get_string(global_cfg, "url-prefix", NULL)) != 0
        || appsvr_iapppay_module_set_free_pay_product_id(iapppay, cfg_get_string(iapppay_cfg, "free-pay-product-id", NULL)) != 0
        || appsvr_iapppay_module_set_app_id(iapppay, cfg_get_string(android_cfg, "app-id", NULL)) != 0
        || appsvr_iapppay_module_set_appv_key(iapppay, cfg_get_string(android_cfg, "appv-key", NULL)) != 0
        || appsvr_iapppay_module_set_platp_key(iapppay, cfg_get_string(android_cfg, "platp-key", NULL)) != 0
        )
    {
        CPE_ERROR(iapppay->m_em, "appsvr_iapppay_module: set config data fail!");
        return -1;
    }

    backend = (appsvr_iapppay_backend_t)mem_alloc(iapppay->m_alloc, sizeof(struct appsvr_iapppay_backend));
    if (backend == NULL) {
        CPE_ERROR(iapppay->m_em, "appsvr_iapppay_backend_init: alloc fail!");
        return -1;
    }
    backend->m_app_env = plugin_app_env_module_find_nc(iapppay->m_app, NULL);
    
    jobject iapppay_cls = env->FindClass("com/drowgames/iapppay/IAppPayUtils");
    if (iapppay_cls == NULL) {
        CPE_ERROR(iapppay->m_em, "appsvr_iapppay_backend_init: get IAppPayUtils class fail!");
        mem_free(iapppay->m_alloc, backend);
        return -1;
    }
    
	backend->m_iapppay_cls = (jclass)env->NewGlobalRef(iapppay_cls);
    if (backend->m_iapppay_cls == NULL) {
        CPE_ERROR(iapppay->m_em, "appsvr_iapppay_backend_init: NewGlobalRef IAppPayUtils class fail!");
        mem_free(iapppay->m_alloc, backend);
        return -1;
    }

    backend->m_init = env->GetStaticMethodID(
        backend->m_iapppay_cls, "init", "(Landroid/app/Activity;JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    assert(backend->m_init);

    backend->m_fini = env->GetStaticMethodID(
        backend->m_iapppay_cls, "fini", "()V");
    assert(backend->m_fini);
    
    backend->m_start_pay = env->GetStaticMethodID(
        backend->m_iapppay_cls, "startPay", "(Landroid/app/Activity;Ljava/lang/String;ILjava/lang/String;FLjava/lang/String;)V");
    assert(backend->m_start_pay);

    jobject context = (jobject)plugin_app_env_android_activity(backend->m_app_env);
    env->CallStaticVoidMethod(
        backend->m_iapppay_cls, backend->m_init,
        context,
        (jlong)iapppay,
        env->NewStringUTF(iapppay->m_app_id),
        env->NewStringUTF(iapppay->m_appv_key),
        env->NewStringUTF(iapppay->m_platp_key));
    if (env->ExceptionCheck()) {
        CPE_ERROR(iapppay->m_em, "appsvr_iapppay_backend_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        
        env->DeleteGlobalRef(backend->m_iapppay_cls);
        mem_free(iapppay->m_alloc, backend);
        return -1;
    }
    
    iapppay->m_backend = backend;

    return 0;
}

void appsvr_iapppay_backend_fini(appsvr_iapppay_module_t iapppay) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_iapppay_backend_t backend = iapppay->m_backend;
    
    assert(backend);

    assert(backend->m_iapppay_cls);
    
    env->CallStaticVoidMethod(backend->m_iapppay_cls, backend->m_fini);
    if (env->ExceptionCheck()) {
        CPE_ERROR(iapppay->m_em, "appsvr_iapppay_backend_fini: call fini fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteGlobalRef(backend->m_iapppay_cls);
    
    mem_free(iapppay->m_alloc, backend);
    iapppay->m_backend = NULL;
}

int appsvr_iapppay_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req) {
    appsvr_iapppay_payment_adapter_t payment_adapter = (appsvr_iapppay_payment_adapter_t)appsvr_payment_adapter_data(adapter);
    appsvr_iapppay_module_t iapppay = payment_adapter->m_module;
    appsvr_iapppay_backend_t backend = iapppay->m_backend;
 	JNIEnv *env = (JNIEnv *)android_jni_env();    
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_app_env);
    
    env->CallStaticVoidMethod(
        backend->m_iapppay_cls, backend->m_start_pay,
        context,
        env->NewStringUTF(req->user_id),
        (jint)(iapppay->m_free_pay_product_id ? atoi(iapppay->m_free_pay_product_id) : req->product_id),
        env->NewStringUTF(req->product_name),
        (jfloat)req->price,
        env->NewStringUTF(req->trade_id));
    if (env->ExceptionCheck()) {
        CPE_ERROR(iapppay->m_em, "appsvr_iapppay_module_pay: call fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    
    return 0;
}

extern "C" {
    
JNIEXPORT void JNICALL Java_com_drowgames_iapppay_IAppPayListener_nativeNotifyResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result, jint service_result, jstring error_msg)
{
    appsvr_iapppay_module_t iapppay = (appsvr_iapppay_module_t)(ptr);
    char * str_error_msg = (char*)env->GetStringUTFChars(error_msg, NULL);
        
    APPSVR_PAYMENT_RESULT payment_result;
    bzero(&payment_result, sizeof(payment_result));
    payment_result.result = (uint8_t)result;
    payment_result.service_result = (int32_t)service_result;
    cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), str_error_msg);

    env->ReleaseStringUTFChars(error_msg, str_error_msg);
    
    appsvr_payment_adapter_notify_result(iapppay->m_adapter, &payment_result);    
}

}
