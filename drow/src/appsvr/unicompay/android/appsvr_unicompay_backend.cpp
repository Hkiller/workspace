#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_unicompay_backend.hpp"
#include "../appsvr_unicompay_payment_adapter_i.h"

int appsvr_unicompay_backend_init(appsvr_unicompay_module_t unicompay) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_unicompay_backend_t backend;

    cfg_t global_cfg = cfg_find_cfg(gd_app_cfg(unicompay->m_app), "args");
    cfg_t unicompay_cfg = cfg_find_cfg(global_cfg, "unicompay");
    cfg_t android_cfg = cfg_find_cfg(unicompay_cfg, "android");

    if (appsvr_unicompay_module_set_chanel(unicompay, cfg_get_string(global_cfg, "chanel", NULL)) != 0
        //|| appsvr_unicompay_module_set_url(unicompay, cfg_get_string(global_cfg, "url-prefix", NULL)) != 0
       // || appsvr_unicompay_module_set_free_pay_product_id(unicompay, cfg_get_string(unicompay_cfg, "free-pay-product-id", NULL)) != 0
        //|| appsvr_unicompay_module_set_app_id(unicompay, cfg_get_string(android_cfg, "app-id", NULL)) != 0
        //|| appsvr_unicompay_module_set_appv_key(unicompay, cfg_get_string(android_cfg, "appv-key", NULL)) != 0
       // || appsvr_unicompay_module_set_platp_key(unicompay, cfg_get_string(android_cfg, "platp-key", NULL)) != 0
        )
    {
        CPE_ERROR(unicompay->m_em, "appsvr_unicompay_module: set config data fail!");
        return -1;
    }

    backend = (appsvr_unicompay_backend_t)mem_alloc(unicompay->m_alloc, sizeof(struct appsvr_unicompay_backend));
    if (backend == NULL) {
        CPE_ERROR(unicompay->m_em, "appsvr_unicompay_backend_init: alloc fail!");
        return -1;
    }
    backend->m_app_env = plugin_app_env_module_find_nc(unicompay->m_app, NULL);
    
    jobject unicompay_cls = env->FindClass("com/drowgames/unicompay/UnicomPayManip");
    if (unicompay_cls == NULL) {
        CPE_ERROR(unicompay->m_em, "appsvr_unicompay_backend_init: get UnicomPayManip class fail!");
        mem_free(unicompay->m_alloc, backend);
        return -1;
    }
    
	backend->m_unicompay_cls = (jclass)env->NewGlobalRef(unicompay_cls);
    if (backend->m_unicompay_cls == NULL) {
        CPE_ERROR(unicompay->m_em, "appsvr_unicompay_backend_init: NewGlobalRef IAppPayUtils class fail!");
        mem_free(unicompay->m_alloc, backend);
        return -1;
    }

    backend->m_init = env->GetStaticMethodID(
        backend->m_unicompay_cls, "init", "(Landroid/app/Activity;J)V");
    assert(backend->m_init);

    backend->m_fini = env->GetStaticMethodID(
        backend->m_unicompay_cls, "fini", "()V");
    assert(backend->m_fini);
    
    backend->m_start_pay_offline = env->GetStaticMethodID(
        backend->m_unicompay_cls, "startPayOffline", "(Landroid/app/Activity;Ljava/lang/String;)V");
    assert(backend->m_start_pay_offline);

    jobject context = (jobject)plugin_app_env_android_activity(backend->m_app_env);
    env->CallStaticVoidMethod(
        backend->m_unicompay_cls, backend->m_init,
        context,
        (jlong)unicompay);
    if (env->ExceptionCheck()) {
        CPE_ERROR(unicompay->m_em, "appsvr_unicompay_backend_init: init fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        
        env->DeleteGlobalRef(backend->m_unicompay_cls);
        mem_free(unicompay->m_alloc, backend);
        return -1;
    }
    
    unicompay->m_backend = backend;

    return 0;
}

void appsvr_unicompay_backend_fini(appsvr_unicompay_module_t unicompay) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    appsvr_unicompay_backend_t backend = unicompay->m_backend;
    
    assert(backend);

    assert(backend->m_unicompay_cls);
    
    env->CallStaticVoidMethod(backend->m_unicompay_cls, backend->m_fini);
    if (env->ExceptionCheck()) {
        CPE_ERROR(unicompay->m_em, "appsvr_unicompay_backend_fini: call fini fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteGlobalRef(backend->m_unicompay_cls);
    
    mem_free(unicompay->m_alloc, backend);
    unicompay->m_backend = NULL;
}

int appsvr_unicompay_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req) {
    appsvr_unicompay_payment_adapter_t payment_adapter = (appsvr_unicompay_payment_adapter_t)appsvr_payment_adapter_data(adapter);
    appsvr_unicompay_module_t unicompay = payment_adapter->m_module;
    appsvr_unicompay_backend_t backend = unicompay->m_backend;
 	JNIEnv *env = (JNIEnv *)android_jni_env();    
    jobject context = (jobject)plugin_app_env_android_activity(backend->m_app_env);
    char chanel_pay_id[64];
    snprintf(chanel_pay_id, sizeof(chanel_pay_id), "%03d", req->chanel_pay_id);

    env->CallStaticVoidMethod(
        backend->m_unicompay_cls, backend->m_start_pay_offline,
        context,
        env->NewStringUTF(chanel_pay_id));
    if (env->ExceptionCheck()) {
        CPE_ERROR(unicompay->m_em, "appsvr_unicompay_module_pay: call fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return 0;
}

extern "C" {
    
JNIEXPORT void JNICALL Java_com_drowgames_unicompay_UnicomOffLinePayListener_nativeNotifyResult(
    JNIEnv *env, jobject obj, jlong ptr, jint result, jint service_result, jstring error_msg)
{
    
    appsvr_unicompay_module_t unicompay = (appsvr_unicompay_module_t)(ptr);
    char * str_error_msg = (char*)env->GetStringUTFChars(error_msg, NULL);

    APPSVR_PAYMENT_RESULT payment_result;
    bzero(&payment_result, sizeof(payment_result));
    payment_result.result = (uint8_t)result;
    payment_result.service_result = (int32_t)service_result;
    cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), str_error_msg);

    env->ReleaseStringUTFChars(error_msg, str_error_msg);
    
    appsvr_payment_adapter_notify_result(unicompay->m_adapter, &payment_result);    
}

}
