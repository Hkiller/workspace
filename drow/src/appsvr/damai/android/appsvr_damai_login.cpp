#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr/account/appsvr_account_adapter.h"
#include "appsvr_damai_backend.hpp"

static int appsvr_damai_account_do_start(appsvr_account_adapter_t adapter, uint8_t is_relogin) {
	appsvr_damai_module_t module = *(appsvr_damai_module_t*)appsvr_account_adapter_data(adapter);
	JNIEnv *env = (JNIEnv *)android_jni_env();

	jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);

	env->CallStaticVoidMethod(
		module->m_backend->m_manip_cls, module->m_backend->m_manip_login,
		context);
	if (env->ExceptionCheck()) {
		CPE_ERROR(module->m_em, "appsvr_damai_login: call loing fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return -1;
	}

	return 0;
}

static int appsvr_damai_account_relogin_start(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_RELOGIN const * req) {
	return appsvr_damai_account_do_start(adapter, 1);
}

static int appsvr_damai_account_login_start(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_LOGIN const * req) {
	return appsvr_damai_account_do_start(adapter, 0);
}

int appsvr_damai_login_init(appsvr_damai_backend_t backend) {
    backend->m_account_adapter = 
        appsvr_account_adapter_create(
            backend->m_module->m_account_module, appsvr_account_damai, "damai",
            sizeof(appsvr_damai_module_t), appsvr_damai_account_login_start, appsvr_damai_account_relogin_start);
    if (backend->m_account_adapter == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_damai_login_init: create adapter fail!");
        return -1;
    }
    
    *(appsvr_damai_module_t*)appsvr_account_adapter_data(backend->m_account_adapter) = backend->m_module;
    
    return 0;
}

void appsvr_damai_login_fini(appsvr_damai_backend_t backend) {
    if (backend->m_account_adapter) {
        appsvr_account_adapter_free(backend->m_account_adapter);
        backend->m_account_adapter = NULL;
    }
}

extern "C" {
    
JNIEXPORT void JNICALL Java_com_drowgames_damai_DamaiSdkManip_nativeNotifyLoginResult(
    JNIEnv *env, jobject obj, jlong ptr, jstring username, jstring access_token, jint error, jstring error_msg)
{
    appsvr_damai_module_t module = (appsvr_damai_module_t)(ptr);

    APPSVR_ACCOUNT_LOGIN_RESULT login_result;
    bzero(&login_result, sizeof(login_result));
    
    if (error == 0) {
        char * username_cstr = (char*)env->GetStringUTFChars(username, NULL);
        char * token_cstr = (char*)env->GetStringUTFChars(access_token, NULL);
        
        login_result.result = 0;
        snprintf(login_result.token, sizeof(login_result.token), "%s+%s", token_cstr, username_cstr);
        login_result.expires_in = 0;
        login_result.error_msg[0] = 0;
        
        env->ReleaseStringUTFChars(username, username_cstr);
        env->ReleaseStringUTFChars(access_token, token_cstr);
    }
    else {
        char * error_msg_cstr = (char*)env->GetStringUTFChars(error_msg, NULL);
        login_result.result = error;
        login_result.token[0] = 0;
        login_result.expires_in = 0;
        cpe_str_dup(login_result.error_msg, sizeof(login_result.error_msg), error_msg_cstr);

        switch(error) {
        default:
            break;
        }
        
        env->ReleaseStringUTFChars(error_msg, error_msg_cstr);
    }
    
    appsvr_account_adapter_notify_login_result(module->m_backend->m_account_adapter, &login_result);    
}

}
