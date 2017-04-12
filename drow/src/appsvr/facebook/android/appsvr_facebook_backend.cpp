#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr_facebook_backend.hpp"
#include "appsvr/account/appsvr_account_adapter.h"

static struct {
    const char * name; 
    int (*init)(appsvr_facebook_backend_t backend);
    void (*fini)(appsvr_facebook_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_facebook_jni_init, appsvr_facebook_jni_fini }
    , { "delegate", appsvr_facebook_delegate_init, appsvr_facebook_delegate_fini }
};

int appsvr_facebook_backend_init(appsvr_facebook_module_t module) {
    appsvr_facebook_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_facebook_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_facebook_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_facebook_backend_create: alloc fail!");
        return -1;
    }

    bzero(backend, sizeof(*backend));
    backend->m_module = module;

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(backend) != 0) {
            CPE_ERROR(module->m_em, "appsvr_facebook_backend_init: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(backend);
            }

            mem_free(module->m_alloc, backend);
            return -1;
        }
    }
    
    CPE_INFO(module->m_em, "appsvr_facebook_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_facebook_backend_fini(appsvr_facebook_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

int appsvr_facebook_backend_login_start(appsvr_facebook_module_t module, uint8_t is_relogin) {
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_manip_login,
        (jboolean)is_relogin);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_facebook_login: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return 0;
}

void appsvr_facebook_on_activity_result(appsvr_facebook_module_t module,uint32_t requestCode, uint32_t resultCode){
    JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_on_activity_result,
        (jint)requestCode,
        (jint)resultCode);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_facebook_on_activity_result: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    CPE_ERROR(module->m_em, "appsvr_facebook_on_activity_result: enter!");
}

void appsvr_facebook_on_suspend(appsvr_facebook_module_t module) {
}

void appsvr_facebook_on_resume(appsvr_facebook_module_t module) {
}

extern "C" {

JNIEXPORT void JNICALL Java_com_drowgames_facebook_FacebookLoginListener_nativeNotifyResult(
    JNIEnv *env, jobject obj, jlong ptr, jint expires_in, jstring access_token, jint error)
{
    appsvr_facebook_module_t module = (appsvr_facebook_module_t)(ptr);
    APPSVR_ACCOUNT_LOGIN_RESULT login_result;
    bzero(&login_result, sizeof(login_result));

    if (error == 0) {
        char * token_cstr = (char*)env->GetStringUTFChars(access_token, NULL);
        login_result.result = 0;
        cpe_str_dup(login_result.token, sizeof(login_result.token), token_cstr);
        login_result.expires_in = (uint32_t)expires_in;
        login_result.error_msg[0] = 0;
        env->ReleaseStringUTFChars(access_token, token_cstr);
    }
    else {
        login_result.result = error;
        login_result.token[0] = 0;
        login_result.expires_in = 0;
        switch(error) {
        default:
            snprintf(login_result.error_msg, sizeof(login_result.error_msg), "error-%d", (int)error);
            break;
        }
    }

    appsvr_facebook_set_token(module, error == 0 ? login_result.token : NULL);
    appsvr_account_adapter_notify_login_result(module->m_account_adapter, &login_result);  
}

}