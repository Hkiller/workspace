#include <android/log.h>
#include <pthread.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "plugin_app_env_backend_i.h"

void android_cpe_error_log_to_log(struct error_info * info, void * context, const char * fmt, va_list args) {
    int prio = ANDROID_LOG_DEBUG;
    switch(info->m_level) {
    case CPE_EL_INFO:
        prio = ANDROID_LOG_INFO;
        break;
    case CPE_EL_WARNING:
        prio = ANDROID_LOG_WARN;
        break;
    case CPE_EL_ERROR:
        prio = ANDROID_LOG_ERROR;
        break;
    }

	__android_log_vprint(prio, "drow", fmt, args);
}

static JavaVM * g_android_jvm = NULL;
char g_android_apk[256] = { 0 };
char g_android_internal_dir[256] = { 0 };
char g_android_external_dir[256] = { 0 };

extern "C"
void android_set_current_apk(const char * full_apk_name) {
    cpe_str_dup(g_android_apk, sizeof(g_android_apk), full_apk_name);
}

extern "C"
const char * android_current_apk(void) {
    return g_android_apk;
}

extern "C"
void android_set_internal_dir(const char * path) {
    cpe_str_dup(g_android_internal_dir, sizeof(g_android_internal_dir), path);
}

extern "C"
const char * android_internal_dir(void) {
    return g_android_internal_dir;
}

extern "C"
void android_set_external_dir(const char * path) {
    cpe_str_dup(g_android_external_dir, sizeof(g_android_external_dir), path);
}

extern "C"
void * android_jvm(void) {
    return (void*)g_android_jvm;
}

extern "C"
void android_set_jvm(void * jvm) {
    g_android_jvm = (JavaVM *)jvm;
}

static pthread_key_t g_android_jni_env_key = 0;

static void android_jni_thread_cleanup(void * ptr) {
    __pthread_cleanup_t* pc = (__pthread_cleanup_t*)ptr;
    free(pc);
    
    if (g_android_jvm) {
        g_android_jvm->DetachCurrentThread();
    }
}

extern "C"
JNIEnv * android_jni_env(void) {
	JNIEnv* env = NULL;
    
    if (g_android_jni_env_key) {
		env = (JNIEnv*)pthread_getspecific(g_android_jni_env_key);
	}
	else {
		pthread_key_create(&g_android_jni_env_key, NULL);
	}

	if (env == NULL) {
		if (g_android_jvm == NULL) {
			__android_log_write(ANDROID_LOG_ERROR, "cpe",  "android_jni_env: no JVM!");
			return NULL;
		}

        int error = g_android_jvm->AttachCurrentThread(&env, NULL);
		if (error || env == NULL) {
			__android_log_write(ANDROID_LOG_ERROR, "cpe",  "android_jni_env: attach thread to JVM!");
			return NULL;
		}

		pthread_setspecific(g_android_jni_env_key, env);

        __pthread_cleanup_t * pc = (__pthread_cleanup_t * )malloc(sizeof(__pthread_cleanup_t));
        __pthread_cleanup_push(pc, &android_jni_thread_cleanup, pc);
	}

    return env;
}

extern "C"
char * plugin_app_env_android_dup_str(char * buf, size_t buf_size, jstring jstr) {
	JNIEnv* env = android_jni_env();
    const char * v = (const char*)env->GetStringUTFChars(jstr, NULL);
    if (v == NULL) return NULL;
    cpe_str_dup(buf, buf_size, v);
    env->ReleaseStringUTFChars(jstr, v);
    return buf;
}

extern "C"
uint8_t plugin_app_env_android_check_exception(error_monitor_t em) {
	JNIEnv* env = android_jni_env();
    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 0;
    }
    else {
        return 1;
    }
}
