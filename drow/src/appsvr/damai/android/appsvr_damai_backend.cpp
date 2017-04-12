#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr_damai_backend.hpp"

static struct {
    const char * name; 
    int (*init)(appsvr_damai_backend_t backend);
    void (*fini)(appsvr_damai_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_damai_jni_init, appsvr_damai_jni_fini }
    , { "login", appsvr_damai_login_init, appsvr_damai_login_fini }
    , { "payment", appsvr_damai_payment_init, appsvr_damai_payment_fini }
};

int appsvr_damai_backend_init(appsvr_damai_module_t module) {
    appsvr_damai_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_damai_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_damai_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_damai_backend_create: alloc fail!");
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
    
    CPE_INFO(module->m_em, "appsvr_damai_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_damai_backend_fini(appsvr_damai_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

void appsvr_damai_on_suspend(appsvr_damai_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(module->m_backend->m_manip_cls, module->m_backend->m_manip_suspend);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "damai: call suspend fail!");
        return;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "damai: call suspend success!");
        }
    }
}

void appsvr_damai_on_resume(appsvr_damai_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(module->m_backend->m_manip_cls, module->m_backend->m_manip_resume);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "damai: call resume fail!");
        return;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "damai: call resume success!");
        }
    }
}

void appsvr_damai_backend_set_userinfo(appsvr_damai_module_t module, const char * userinfo) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();

    env->CallStaticVoidMethod(module->m_backend->m_manip_cls, module->m_backend->m_manip_set_userinfo, env->NewStringUTF(userinfo));
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "damai: set userinfo fail!");
        return;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "damai: set userinfo success!");
        }
    }
}
