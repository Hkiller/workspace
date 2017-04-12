#include <cassert>
#include "plugin_app_env_backend_i.h"
#include "plugin_app_vfs_backend_i.h"
#include "plugin_app_env_delegate.h"

static struct {
    const char * name;
    int (*init)(plugin_app_env_backend_t module);
    void (*fini)(plugin_app_env_backend_t module);
} s_auto_reg_products[] = {
    { "vfs-backend", ui_app_android_vfs_backend_init, ui_app_android_vfs_backend_fini }
    , { "vfs-mount", ui_app_android_vfs_backend_mount, ui_app_android_vfs_backend_unmount }
};

extern "C"
int plugin_app_env_android_set_activity(plugin_app_env_module_t module, void * activity) {
    plugin_app_env_backend_t backend = (plugin_app_env_backend_t)module->m_backend;
    JNIEnv * jni_env = (JNIEnv *)android_jni_env();

    if (backend->m_activity) {
        jni_env->DeleteGlobalRef((jobject)backend->m_activity);
    }

    if (activity) {
        backend->m_activity = jni_env->NewGlobalRef((jobject)activity);
        if (backend->m_activity == NULL) {
            CPE_ERROR(module->m_em, "plugin_app_env_android_set_activity: NewGlobalRef fail!");
            return -1;
        }
    }
    else {
        backend->m_activity = NULL;
    }

    return 0;
}

extern "C"
void * plugin_app_env_android_activity(plugin_app_env_module_t module) {
    plugin_app_env_backend_t backend = (plugin_app_env_backend_t)module->m_backend;
    return backend->m_activity;
}

extern "C"
jclass plugin_app_env_android_find_class(plugin_app_env_module_t module, const char * class_name) {
    plugin_app_env_backend_t backend = (plugin_app_env_backend_t)module->m_backend;
    JNIEnv * env = (JNIEnv *)android_jni_env();

    if (backend->m_find_class_mid == NULL) {
        assert(backend->m_dex_loader == NULL);
        
        /*找到ClassLoader类 */
        jclass loader_cls = env->FindClass("java/lang/ClassLoader");
        if(loader_cls == NULL) {
            CPE_ERROR(module->m_em, "plugin_app_env_backend_find_class: find java/lang/ClassLoader fail!");
            return NULL;
        }
        
        jobject loader = env->CallStaticObjectMethod(loader_cls, env->GetStaticMethodID(loader_cls, "getSystemClassLoader","()Ljava/lang/ClassLoader;"));
        if(loader == NULL) {
            CPE_ERROR(module->m_em, "plugin_app_env_backend_find_class: getSystemClassLoader fail!");
            return NULL;
        }

        /*jar包存放位置 */
        jstring dexpath = env->NewStringUTF(android_internal_dir());
        jstring dex_odex_path = env->NewStringUTF(android_internal_dir());

        /*新建一个DexClassLoader对象 */
        jclass dex_loader_cls = env->FindClass("dalvik/system/DexClassLoader");
        if(dex_loader_cls == NULL) {
            CPE_ERROR(module->m_em, "plugin_app_env_backend_find_class: find dalvik/system/DexClassLoader fail!");
            return NULL;
        }

        jobject dex_loader =
            env->NewObject(
                dex_loader_cls,
                env->GetMethodID(dex_loader_cls, "<init>","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V"),
                dexpath, dex_odex_path, NULL, loader);
        if (dex_loader == NULL) {
            CPE_ERROR(module->m_em, "plugin_app_env_backend_find_class: create DexClassLoader fail!");
            return NULL;
        }
        backend->m_dex_loader = env->NewGlobalRef(dex_loader);
        if (backend->m_dex_loader == NULL) {
            CPE_ERROR(module->m_em, "plugin_app_env_backend_find_class: NewGlobalRef DexClassLoader fail!");
            return NULL;
        }
        
        /*找到DexClassLoader中的方法findClass */
        backend->m_find_class_mid = env->GetMethodID(dex_loader_cls, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        if(backend->m_find_class_mid == NULL) {
            backend->m_find_class_mid = env->GetMethodID(dex_loader_cls,"loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
            if (backend->m_find_class_mid == NULL) {
                CPE_ERROR(module->m_em, "plugin_app_env_backend_find_class: get findClass or loadClass function fail!");
                env->DeleteGlobalRef(backend->m_dex_loader);
                backend->m_dex_loader = NULL;
                return NULL;
            }
        }
    }

    //存储需要调用的类
    jstring js_class_name = env->NewStringUTF(class_name);
    jclass client_class = (jclass)env->CallObjectMethod(backend->m_dex_loader, backend->m_find_class_mid, js_class_name);
    if (client_class == NULL) {
        CPE_ERROR(module->m_em, "plugin_app_env_backend_find_class: find class %s fail!", class_name);
        return NULL;
    }

    return client_class;
}

extern "C"
int plugin_app_env_backend_init(plugin_app_env_module_t module) {
    plugin_app_env_backend_t backend = (plugin_app_env_backend_t)mem_alloc(module->m_alloc, sizeof(struct plugin_app_env_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "plugin_app_env_backend_init: alloc fail!");
        return -1;
    }
    backend->m_module = module;
    backend->m_activity = NULL;
    backend->m_dex_loader = NULL;
    backend->m_find_class_mid = NULL;
    TAILQ_INIT(&backend->m_delegators);

    for(uint8_t component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(backend) != 0) {
            CPE_ERROR(module->m_em, "plugin_app_env_backend_init: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(backend);
            }
            mem_free(module->m_alloc, backend);
            return -1;
        }
    }

    module->m_backend = backend;
    
    return 0;
}

extern "C"
void plugin_app_env_backend_fini(plugin_app_env_module_t module) {
    plugin_app_env_backend_t backend = (plugin_app_env_backend_t)module->m_backend;
    JNIEnv * jni_env = (JNIEnv *)android_jni_env();

    assert(TAILQ_EMPTY(&backend->m_delegators));

    for(uint8_t component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(backend);
    }

    if (backend->m_activity) {
        jni_env->DeleteGlobalRef(backend->m_activity);
        backend->m_activity = NULL;
    }

    if (backend->m_dex_loader) {
        jni_env->DeleteGlobalRef(backend->m_dex_loader);
        backend->m_dex_loader = NULL;
        backend->m_find_class_mid = NULL;
    }
    
    mem_free(module->m_alloc, module->m_backend);
    module->m_backend = NULL;
}
