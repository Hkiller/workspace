#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "appsvr_weixin_backend.hpp"

static struct {
    const char * name; 
    int (*init)(appsvr_weixin_backend_t backend);
    void (*fini)(appsvr_weixin_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_weixin_jni_init, appsvr_weixin_jni_fini }
};

int appsvr_weixin_backend_init(appsvr_weixin_module_t module) {
    appsvr_weixin_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_weixin_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_weixin_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_weixin_backend_create: alloc fail!");
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
    
    CPE_INFO(module->m_em, "appsvr_weixin_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_weixin_backend_fini(appsvr_weixin_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

int appsvr_weixin_backend_start_login(appsvr_weixin_module_t module, uint8_t is_relogin) {
	JNIEnv *env = (JNIEnv *)android_jni_env();
    char session_buf[32];

    snprintf(session_buf, sizeof(session_buf), "%d", module->m_login_session);
    
    jstring state = env->NewStringUTF(session_buf);
    jstring scope = env->NewStringUTF(module->m_scope);
    
	env->CallStaticVoidMethod(
		module->m_backend->m_manip_cls, module->m_backend->m_manip_login, scope, state);
	if (env->ExceptionCheck()) {
		CPE_ERROR(module->m_em, "appsvr_weixin_login: call loing fail!");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return -1;
	}

	return 0;
}


extern "C" {
    
JNIEXPORT void JNICALL Java_com_drowgames_weixin_WeixinManip_nativeNotifyResult(
    JNIEnv *env, jobject obj, jlong ptr, jstring access_token, jstring state, jint error, jstring errormsg)
{
    appsvr_weixin_module_t module = (appsvr_weixin_module_t)(ptr);
    
    char * c_access_token = access_token ? (char*)env->GetStringUTFChars(access_token, NULL) : NULL;    
    char * c_errormsg = errormsg ? (char*)env->GetStringUTFChars(errormsg, NULL) : NULL;
    char * c_state = state ? (char*)env->GetStringUTFChars(state, NULL) : NULL;

    uint32_t session = c_state ? atoi(c_state) : 0;
    appsvr_weixin_notify_login_result(module, c_access_token, session, error, c_errormsg);

    if (c_access_token) env->ReleaseStringUTFChars(access_token, c_access_token);
    if (c_errormsg) env->ReleaseStringUTFChars(errormsg, c_errormsg);
    if (c_state) env->ReleaseStringUTFChars(state, c_state);
}

}
