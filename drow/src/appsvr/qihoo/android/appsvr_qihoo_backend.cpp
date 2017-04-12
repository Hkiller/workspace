#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr_qihoo_backend.hpp"

static struct {
    const char * name; 
    int (*init)(appsvr_qihoo_backend_t backend);
    void (*fini)(appsvr_qihoo_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_qihoo_jni_init, appsvr_qihoo_jni_fini }
    , { "login", appsvr_qihoo_login_init, appsvr_qihoo_login_fini }
    , { "payment", appsvr_qihoo_payment_init, appsvr_qihoo_payment_fini }
};

int appsvr_qihoo_backend_init(appsvr_qihoo_module_t module) {
    appsvr_qihoo_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_qihoo_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_qihoo_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_qihoo_backend_create: alloc fail!");
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
    
    CPE_INFO(module->m_em, "appsvr_qihoo_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_qihoo_backend_fini(appsvr_qihoo_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

void appsvr_qihoo_on_pause(appsvr_qihoo_module_t module) {
 	// JNIEnv *env = (JNIEnv *)android_jni_env();
    // jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    // assert(context);

    // env->CallStaticVoidMethod(module->m_backend->m_agent_cls, module->m_backend->m_agent_on_pause, context);
    // if (env->ExceptionCheck()) {
    //     CPE_ERROR(module->m_em, "qihoo: call onPause fail!");
    //     return;
    // }
    // else {
    //     if (module->m_debug) {
    //         CPE_INFO(module->m_em, "qihoo: call onPause success!");
    //     }
    // }
}

void appsvr_qihoo_on_resume(appsvr_qihoo_module_t module) {
 	// JNIEnv *env = (JNIEnv *)android_jni_env();
    // jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    // assert(context);

    // env->CallStaticVoidMethod(module->m_backend->m_agent_cls, module->m_backend->m_agent_on_resume, context);
    // if (env->ExceptionCheck()) {
    //     CPE_ERROR(module->m_em, "qihoo: call onResume fail!");
    //     return;
    // }
    // else {
    //     if (module->m_debug) {
    //         CPE_INFO(module->m_em, "qihoo: call onResume success!");
    //     }
    // }
}
