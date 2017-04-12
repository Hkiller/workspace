#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr/account/appsvr_account_adapter.h"
#include "appsvr_qihoo_backend.hpp"

static int appsvr_qihoo_account_do_start(appsvr_account_adapter_t adapter, uint8_t is_relogin) {
	appsvr_qihoo_module_t module = *(appsvr_qihoo_module_t*)appsvr_account_adapter_data(adapter);
	JNIEnv *env = (JNIEnv *)android_jni_env();

	jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
	uint8_t isShowClose = 0;
	uint8_t isShowSwitchButton = 1;
	uint8_t isHideWellcome = 1;
	uint8_t isNeedActivationCode = 0;
	uint8_t isAutoLoginHideUI = 1;
	uint8_t isShowDlgOnFailedAutoLogin = 1;

    const char * background = module->m_background ? module->m_background : "";

	env->CallStaticVoidMethod(
		module->m_backend->m_manip_cls, module->m_backend->m_manip_login,
		(jlong)(ptr_int_t)(module),
		context,
		(jboolean)module->m_land_space,
		(jboolean)isShowClose,
		(jboolean)isShowSwitchButton,
		(jboolean)isHideWellcome,
		env->NewStringUTF(background),
		(jboolean)(background[0] == '/' ? 1 : 0),
		(jboolean)isNeedActivationCode,
		(jboolean)isAutoLoginHideUI,
		(jboolean)isShowDlgOnFailedAutoLogin,
		(jboolean)is_relogin);
	if (env->ExceptionCheck()) {
		CPE_ERROR(module->m_em, "appsvr_qihoo_login: call loing fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return -1;
	}

	return 0;
}

static int appsvr_qihoo_account_relogin_start(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_RELOGIN const * req) {
	return appsvr_qihoo_account_do_start(adapter, 1);
}

static int appsvr_qihoo_account_login_start(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_LOGIN const * req) {
	return appsvr_qihoo_account_do_start(adapter, 0);
}

int appsvr_qihoo_login_init(appsvr_qihoo_backend_t backend) {
    backend->m_account_adapter = 
        appsvr_account_adapter_create(
            backend->m_module->m_account_module, appsvr_account_360, "qihoo",
            sizeof(appsvr_qihoo_module_t), appsvr_qihoo_account_login_start, appsvr_qihoo_account_relogin_start);
    if (backend->m_account_adapter == NULL) {
        CPE_ERROR(backend->m_module->m_em, "appsvr_qihoo_login_init: create adapter fail!");
        return -1;
    }
    
    *(appsvr_qihoo_module_t*)appsvr_account_adapter_data(backend->m_account_adapter) = backend->m_module;
    
    return 0;
}

void appsvr_qihoo_login_fini(appsvr_qihoo_backend_t backend) {
    if (backend->m_account_adapter) {
        appsvr_account_adapter_free(backend->m_account_adapter);
        backend->m_account_adapter = NULL;
    }
}

extern "C" {
    
JNIEXPORT void JNICALL Java_com_drowgames_qihoo_QihooLoginListener_nativeNotifyResult(
    JNIEnv *env, jobject obj, jlong ptr, jint expires_in, jstring access_token, jint error)
{
    appsvr_qihoo_module_t module = (appsvr_qihoo_module_t)(ptr);

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
    
	appsvr_qihoo_set_token(module, error == 0 ? login_result.token : NULL);

    appsvr_account_adapter_notify_login_result(module->m_backend->m_account_adapter, &login_result);    
}

}
