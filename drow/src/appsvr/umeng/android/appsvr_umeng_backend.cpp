#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr_umeng_backend.hpp"
#include "../appsvr_umeng_executor.h"
#include "../appsvr_umeng_pay_chanel.h"

static struct {
    const char * name; 
    int (*init)(appsvr_umeng_backend_t backend);
    void (*fini)(appsvr_umeng_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_umeng_jni_init, appsvr_umeng_jni_fini }
    , { "agent", appsvr_umeng_agent_init, appsvr_umeng_agent_fini }
};

int appsvr_umeng_backend_init(appsvr_umeng_module_t module) {
    appsvr_umeng_backend_t backend;
    uint16_t component_pos;
    
    backend = (appsvr_umeng_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_umeng_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_umeng_backend_create: alloc fail!");
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

    CPE_INFO(module->m_em, "appsvr_umeng_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_umeng_backend_fini(appsvr_umeng_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }
    
    mem_free(module->m_alloc, module->m_backend);
}

void appsvr_umeng_on_pause(appsvr_umeng_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    assert(context);

    env->CallStaticVoidMethod(module->m_backend->m_agent_cls, module->m_backend->m_agent_on_pause, context);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "umeng: call onPause fail!");
        return;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "umeng: call onPause success!");
        }
    }
}

void appsvr_umeng_on_resume(appsvr_umeng_module_t module) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    assert(context);

    env->CallStaticVoidMethod(module->m_backend->m_agent_cls, module->m_backend->m_agent_on_resume, context);
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "umeng: call onResume fail!");
        return;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "umeng: call onResume success!");
        }
    }
}

int appsvr_umeng_executor_backend_init(appsvr_umeng_executor_t executor) {
    appsvr_umeng_module_t module = executor->m_module;
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jmethodID * mid = (jmethodID*)executor->m_backend_data;

    if (strcmp(executor->m_op_name, "onEvent") == 0) {
        *mid = module->m_backend->m_agent_on_event;
    }
    else if (strcmp(executor->m_op_name, "reportError") == 0) {
        if (appsvr_umeng_jni_load_method(
                module->m_backend, env, *mid, true, module->m_backend->m_agent_cls, executor->m_op_name, "(Landroid/content/Context;Ljava/lang/String;)V")
            != 0)
        {
            CPE_ERROR(module->m_em, "appsvr_executor_create: %s: load method fail!", executor->m_op_name);
            return -1;
        }
    }
    else {
        char sign[64];
        size_t n;
        uint8_t i;
        n = snprintf(sign, sizeof(sign), "(");
        for(i = 0; i < executor->m_arg_count; i++) {
            appsvr_umeng_executor_arg_t arg = &executor->m_args[i];
            switch(arg->m_type) {
            case 'S':
                n += snprintf(sign + n, sizeof(sign) - n, "Ljava/lang/String;");
                break;
            case 'I':
                n += snprintf(sign + n, sizeof(sign) - n, "I");
                break;
            case 'D':
                n += snprintf(sign + n, sizeof(sign) - n, "D");
                break;
            default:
                CPE_ERROR(
                    module->m_em, "appsvr_executor_create: %s: arg %d format %d unknown!",
                    executor->m_op_name, i, arg->m_type);
                return -1;
            }
        }
        n += snprintf(sign + n, sizeof(sign) - n, ")V");

        if (appsvr_umeng_jni_load_method(
                module->m_backend, env, *mid, true, module->m_backend->m_agent_cls, executor->m_op_name, sign)
            != 0)
        {
            CPE_ERROR(module->m_em, "appsvr_executor_create: %s: load method fail!", executor->m_op_name);
            return -1;
        }
    }
    
    return 0;
}

struct appsvr_umeng_visit_attrs_ctx {
 	JNIEnv *m_env;
    jobject m_attrs; 
    appsvr_umeng_backend_t m_backend;
};
    
static void appsvr_umeng_visit_attrs(void * input_ctx, const char * value) {
    struct appsvr_umeng_visit_attrs_ctx * ctx = (struct appsvr_umeng_visit_attrs_ctx *)input_ctx;
    appsvr_umeng_module_t module = ctx->m_backend->m_module;
    const char * sep;
    char buf[32];
    const char * k;
    const char * v;
    
    value = cpe_str_trim_head((char*)value);
    
    sep = strchr(value, '=');
    if (sep == NULL) {
        CPE_ERROR(module->m_em, "umeng: parse attr %s fail(no =)!", value);
        return;
    }

    v = cpe_str_trim_head((char*)(sep + 1));

    sep = cpe_str_trim_tail((char*)sep, (char *)value);

    k = cpe_str_dup_range(buf, sizeof(buf), value, sep);
    if (k == NULL) {
        CPE_ERROR(module->m_em, "umeng: parse attr %s fail(key overvlow)!", value);
        return;
    }

    ctx->m_env->CallObjectMethod(ctx->m_attrs, ctx->m_backend->m_map_put, ctx->m_env->NewStringUTF(k), ctx->m_env->NewStringUTF(v));
    if (ctx->m_env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "umeng: parse attr %s: set fail!", value);
    }
}

void appsvr_umeng_executor_backend_exec(appsvr_umeng_executor_t executor, dr_data_source_t data_source) {
    appsvr_umeng_module_t module = executor->m_module;
    appsvr_umeng_backend_t backend = module->m_backend;
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jmethodID * mid = (jmethodID*)executor->m_backend_data;

    if (strcmp(executor->m_op_name, "onProfileSignIn") == 0) {
        assert(executor->m_arg_count == 2);
        env->CallStaticVoidMethod(
            backend->m_agent_cls, *mid,
            env->NewStringUTF(
                dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "")),
            env->NewStringUTF(
                dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[1].m_def, data_source, ""))
            );
    }
    else if (strcmp(executor->m_op_name, "use") == 0) {
        assert(executor->m_arg_count == 3);
        env->CallStaticVoidMethod(
            backend->m_agent_cls, *mid,
            env->NewStringUTF(
                dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "")),
            (jint)dr_calc_int64_with_dft(module->m_computer, executor->m_args[1].m_def, data_source, 0),
            (jdouble)dr_calc_double_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0.0)
            );
    }
    else if (strcmp(executor->m_op_name, "buy") == 0) {
        assert(executor->m_arg_count == 3);
        env->CallStaticVoidMethod(
            backend->m_agent_cls, *mid,
            env->NewStringUTF(
                dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "")),
            (jint)dr_calc_int64_with_dft(module->m_computer, executor->m_args[1].m_def, data_source, 0),
            (jdouble)dr_calc_double_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0.0)
            );
    }
    else if (strcmp(executor->m_op_name, "pay") == 0) {
        assert(executor->m_arg_count == 5);

        uint8_t pay_service = dr_calc_uint8_with_dft(module->m_computer, executor->m_args[4].m_def, data_source, 0);
        if (pay_service == 0) {
            CPE_ERROR(module->m_em, "umeng: call %s: %s not configured!", executor->m_op_name, executor->m_args[4].m_def);
            return;
        }
        
        appsvr_umeng_pay_chanel_t pay_chanel = appsvr_umeng_pay_chanel_find(module, pay_service);
        if (pay_chanel == NULL) {
            CPE_ERROR(module->m_em, "umeng: call %s: chanel %d unknown!", executor->m_op_name, pay_service);
            return;
        }

        const char * item = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[1].m_def, data_source, "");
        if (item[0]) {
            env->CallStaticVoidMethod(
                backend->m_agent_cls, *mid,
                (jdouble)dr_calc_double_with_dft(module->m_computer, executor->m_args[0].m_def, data_source, 0.0),
                env->NewStringUTF(item),
                (jint)dr_calc_int64_with_dft(module->m_computer, executor->m_args[1].m_def, data_source, 0),
                (jdouble)dr_calc_double_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0.0),
                (jint)pay_chanel->m_id
                );
        }
        else {
            jmethodID mid2;
            if (appsvr_umeng_jni_load_method(module->m_backend, env, mid2, true, module->m_backend->m_agent_cls, "pay", "(DDI)V") != 0) {
                CPE_ERROR(module->m_em, "umeng: call %s: get method pay(3) fail!", executor->m_op_name);
                return;
            }
            env->CallStaticVoidMethod(
                backend->m_agent_cls, mid2,
                (jdouble)dr_calc_float_with_dft(module->m_computer, executor->m_args[0].m_def, data_source, 0.0),
                (jdouble)dr_calc_float_with_dft(module->m_computer, executor->m_args[3].m_def, data_source, 0.0),
                (jint)pay_chanel->m_id);

            CPE_ERROR(
                module->m_em, "umeng: call %s: %s=%f, %s=%f, %d!",
                executor->m_op_name,
                executor->m_args[0].m_def,
                dr_calc_float_with_dft(module->m_computer, executor->m_args[0].m_def, data_source, 0.0),
                executor->m_args[3].m_def,
                dr_calc_float_with_dft(module->m_computer, executor->m_args[3].m_def, data_source, 0.0),
                pay_chanel->m_id);
        }
    }
    else if (strcmp(executor->m_op_name, "bonus") == 0) {
        const char * item_name;

        assert(executor->m_arg_count == 4);

        item_name = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "");
        if (item_name[0]) {
            env->CallStaticVoidMethod(
                backend->m_agent_cls, *mid,
                env->NewStringUTF(item_name),
                (jint)dr_calc_int64_with_dft(module->m_computer, executor->m_args[1].m_def, data_source, 0),
                (jdouble)dr_calc_double_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0.0),
                (jint)dr_calc_int64_with_dft(module->m_computer, executor->m_args[3].m_def, data_source, 0)
                );
        }
        else {
            jmethodID mid2;
            if (appsvr_umeng_jni_load_method(module->m_backend, env, mid2, true, module->m_backend->m_agent_cls, "bonus", "(DI)V") != 0) {
                CPE_ERROR(module->m_em, "umeng: call %s: get method bouns fail!", executor->m_op_name);
                return;
            }
            env->CallStaticVoidMethod(
                backend->m_agent_cls, mid2,
                (jdouble)dr_calc_double_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0.0),
                (jint)dr_calc_int64_with_dft(module->m_computer, executor->m_args[3].m_def, data_source, 0)
                );
        }
    }
    else if (strcmp(executor->m_op_name, "onEvent") == 0) {
        jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);

        jstring id =
            env->NewStringUTF(
                dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, ""));

        jobject attrs = env->NewObject(backend->m_map_cls, backend->m_map_init, "");

        const char * count = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[1].m_def, data_source, NULL);
        if (count) {
            env->CallObjectMethod(attrs, backend->m_map_put, env->NewStringUTF("__ct__"), env->NewStringUTF(count));
        }

        const char * str_attrs = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[2].m_def, data_source, NULL);
        if (str_attrs) {
            struct appsvr_umeng_visit_attrs_ctx ctx = { env, attrs, backend };
            cpe_str_list_for_each(str_attrs, ',', appsvr_umeng_visit_attrs, &ctx);
        }

        env->CallStaticVoidMethod(backend->m_agent_cls, *mid, context, id, attrs);
    }
    else if (strcmp(executor->m_op_name, "reportError") == 0) {
        jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);

        env->CallStaticVoidMethod(
            backend->m_agent_cls, *mid, context,
            env->NewStringUTF(
                dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "")));
    }
    else {
        if (executor->m_arg_count == 0) {
            env->CallStaticVoidMethod(backend->m_agent_cls, *mid);
        }
        else if (executor->m_arg_count == 1) {
            struct appsvr_umeng_executor_arg * arg = &executor->m_args[0];
        
            switch(arg->m_type) {
            case 'S':
                env->CallStaticVoidMethod(
                    backend->m_agent_cls, *mid,
                    env->NewStringUTF(
                        dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, arg->m_def, data_source, "")));
                break;
            case 'I':
                env->CallStaticVoidMethod(
                    backend->m_agent_cls, *mid,
                    (jint)dr_calc_int64_with_dft(module->m_computer, arg->m_def, data_source, (int64_t)0));
                break;
            case 'D':
                env->CallStaticVoidMethod(
                    backend->m_agent_cls, *mid,
                    (jdouble)dr_calc_double_with_dft(module->m_computer, arg->m_def, data_source, 0.0));
                break;
            default:
                assert(0);
            }
        }
        else {
            CPE_ERROR(module->m_em, "umeng: call %s not support!", executor->m_op_name);
            return;
        }
    }
    
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "umeng: %s ==> %s fail!", dr_meta_name(data_source->m_data.m_meta), executor->m_op_name);
        return;
    }
    else if (module->m_debug) {
        CPE_INFO(module->m_em, "umeng: %s ==> %s success!", dr_meta_name(data_source->m_data.m_meta), executor->m_op_name);
    }
}

void appsvr_umeng_executor_backend_fini(appsvr_umeng_executor_t executor) {
}
    
void appsvr_umeng_on_page_begin(appsvr_umeng_module_t module, const char * page_name) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    assert(context);

    env->CallStaticVoidMethod(
        module->m_backend->m_agent_cls, module->m_backend->m_agent_on_page_start,
        env->NewStringUTF(page_name));
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "umeng: call onPageStart fail!");
        return;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "umeng: call onPageStart %s success!", page_name);
        }
    }
}

void appsvr_umeng_on_page_end(appsvr_umeng_module_t module, const char * page_name) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    assert(context);

    env->CallStaticVoidMethod(
        module->m_backend->m_agent_cls, module->m_backend->m_agent_on_page_end,
        env->NewStringUTF(page_name));
    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "umeng: call onPageEnd fail!");
        return;
    }
    else {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "umeng: call onPageEnd %s success!", page_name);
        }
    }
}

void appsvr_umeng_on_event(appsvr_umeng_module_t module, const char * str_id, uint32_t count, const char * str_attrs) {
    appsvr_umeng_backend_t backend = module->m_backend;
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    char buf[32];

    jstring id = env->NewStringUTF(str_id);

    jobject attrs = env->NewObject(backend->m_map_cls, backend->m_map_init, "");

    snprintf(buf, sizeof(buf), "%d", count);
    env->CallObjectMethod(attrs, backend->m_map_put, env->NewStringUTF("__ct__"), env->NewStringUTF(buf));

    if (str_attrs) {
        struct appsvr_umeng_visit_attrs_ctx ctx = { env, attrs, backend };
        cpe_str_list_for_each(str_attrs, ',', appsvr_umeng_visit_attrs, &ctx);
    }

    env->CallStaticVoidMethod(module->m_backend->m_agent_cls, module->m_backend->m_agent_on_event, context, id, attrs);
}
