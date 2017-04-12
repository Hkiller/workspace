#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include "cpe/pal/pal_stat.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "appsvr_device_backend.hpp"
#include "../appsvr_device_executor_i.h"

void appsvr_device_start_install(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    appsvr_device_module_t module = (appsvr_device_module_t)ctx;
    APPSVR_DEVICE_START_INSTALL const * req =  (APPSVR_DEVICE_START_INSTALL const *)req_data;
    JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);

    char path_buf[sizeof(req->source)];
    cpe_str_dup(path_buf, sizeof(path_buf), req->source);

    int expect_state = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    while(path_buf[0]) {
        struct stat state_buf;
        if (stat(path_buf, &state_buf) != 0) {
            CPE_ERROR(module->m_em, "appsvr_device_start_install: get state of %s fail, errno=%d (%s)!", path_buf, errno, strerror(errno));
            return;
        }

        int state = state_buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
        if ((state & expect_state) != expect_state) {
            if (chmod(path_buf, state | expect_state) != 0) {
                if (errno == EPERM || errno == EACCES) {
                    break;
                }
                else {
                    CPE_ERROR(module->m_em, "appsvr_device_start_install: chmod of %s fail, errno=%d (%s)!", path_buf, errno, strerror(errno));
                    return;
                }
            }
        }

        char * sep = strrchr(path_buf, '/');
        if (sep == NULL || sep == path_buf) break;
        *sep = 0;
    }
    
    env->CallStaticVoidMethod(
        module->m_backend->m_device_cls, module->m_backend->m_start_install,
        context, env->NewStringUTF(req->source));
}

void appsvr_device_backend_set_path_info(
    appsvr_device_module_t module, APPSVR_DEVICE_QUERY_PATH const * req, APPSVR_DEVICE_PATH * path_info)
{
    path_info->category = req->category;
    cpe_str_dup(path_info->path, sizeof(path_info->path), "");
}

void appsvr_device_backend_set_device_info(appsvr_device_module_t module, APPSVR_DEVICE_INFO * device_info) {
    appsvr_device_backend_t backend = module->m_backend;
    JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);

    device_info->category = appsvr_device_android;
    cpe_str_dup(device_info->cpu_name, sizeof(device_info->cpu_name), "");

    jobject j_device_id = (jobject)env->CallStaticObjectMethod(backend->m_device_cls, backend->m_get_device_id, context);
    if (!plugin_app_env_android_check_exception(module->m_em) || j_device_id == NULL) {
        CPE_ERROR(module->m_em, "appsvr_device_query_info: query device id fail, pleace check right");
        device_info->device_id[0] = 0;
    }
    else {
        plugin_app_env_android_dup_str(device_info->device_id, sizeof(device_info->device_id), (jstring)j_device_id);
    }

    jobject j_device_model = (jobject)env->CallStaticObjectMethod(backend->m_device_cls, backend->m_get_device_model);
    if (!plugin_app_env_android_check_exception(module->m_em) || j_device_model == NULL) {
        CPE_ERROR(module->m_em, "appsvr_device_query_info: query device mode fail, pleace check right");
        device_info->device_model[0] = 0;
    }
    else {
        plugin_app_env_android_dup_str(device_info->device_model, sizeof(device_info->device_model), (jstring)j_device_model);
    }

    jobject j_device_language = (jobject)env->CallStaticObjectMethod(backend->m_device_cls, backend->m_get_language, context);
    if (!plugin_app_env_android_check_exception(module->m_em) || j_device_language == NULL) {
        CPE_ERROR(module->m_em, "appsvr_device_query_info: query device language fail, pleace check right");
        device_info->device_language[0] = 0;
    }
    else {
        plugin_app_env_android_dup_str(device_info->device_language, sizeof(device_info->device_language), (jstring)j_device_language);
    }
    
    device_info->cpu_freq = (uint64_t)env->CallStaticIntMethod(backend->m_device_cls, backend->m_get_cpu_freq);
    device_info->memory_kb = (uint64_t)env->CallStaticLongMethod(backend->m_device_cls, backend->m_get_memory);

    if (device_info->cpu_freq >= 1500000 && device_info->memory_kb >= 1500000) {
        device_info->device_cap = appsvr_device_cap_high;
    }
    else if (device_info->cpu_freq >= 1000000 && device_info->memory_kb >= 1000000) {
        device_info->device_cap = appsvr_device_cap_medium;
    }
    else {
        device_info->device_cap = appsvr_device_cap_low;
    }
}

int appsvr_device_backend_set_network_state(appsvr_device_module_t module, APPSVR_DEVICE_NETWORK_INFO * network_info) {
    JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject context = (jobject)plugin_app_env_android_activity(module->m_app_env);
    
    jint state = env->CallStaticIntMethod(module->m_backend->m_device_cls, module->m_backend->m_get_network_state, context);
    if (!plugin_app_env_android_check_exception(module->m_em)) {
        return -1;
    }

    network_info->state = state;
    return 0;
}
