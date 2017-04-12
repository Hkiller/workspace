#include <cassert>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "AppContext.hpp"

extern "C" {
    int plugin_app_env_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg);
    void plugin_app_env_module_app_fini(gd_app_context_t app, gd_app_module_t module);
}

namespace Ui { namespace App {

AppContext::AppContext()
    : m_app(NULL)
    , m_activity_cls(NULL)
    , m_render_commit_begin(NULL)
    , m_render_commit_done(NULL)
{
    cpe_error_monitor_init(&m_em, android_cpe_error_log_to_log, NULL);
}

int AppContext::init(jobject activity) {
    char prog_name[64] = { 0 };
    const char * apk_name = android_current_apk();

    assert(activity);
    
    if (const char * end = strrchr(apk_name, '.')) {
        const char * start = end;
        while(start > apk_name && *(start - 1) != '.') --start;

        assert(start >= apk_name);
        assert(*start != '.');
        cpe_str_dup_range(prog_name, sizeof(prog_name), start, end);
    }
    else {
        cpe_str_dup(prog_name, sizeof(prog_name), apk_name);
    }
    
    char * argv[] = { prog_name };

    m_app = gd_app_context_create_main(NULL, 0, 1, argv);
    if (m_app == NULL) {
        assert(0);
        return -1;
    }

    gd_app_set_em(m_app, &m_em);
    gd_app_set_debug(m_app, 1);
    gd_app_ins_set(m_app);

    if (gd_app_add_tag(m_app, "android") != 0) {
        APP_CTX_ERROR(m_app, "AppContext: set app_env module tag fail!");
        gd_app_context_free(m_app);
        m_app = NULL;
        return -1;
    }
    
    if (gd_app_module_type_init(
            "plugin_app_env_module",
            plugin_app_env_module_app_init, plugin_app_env_module_app_fini, NULL, NULL, &m_em)
        != 0
        || gd_app_install_module(m_app, "plugin_app_env_module", "plugin_app_env_module", NULL, NULL)
        == NULL)
    {
        APP_CTX_ERROR(m_app, "AppContext: create app_env module fail!");
        gd_app_context_free(m_app);
        m_app = NULL;
        return -1;
    }
    APP_CTX_INFO(m_app, "AppContext: create app %s!", prog_name);

    if (plugin_app_env_android_set_activity(plugin_app_env_module_find_nc(m_app, NULL), activity) != 0) {
        APP_CTX_ERROR(m_app, "AppContext: set activity fail!");
        gd_app_context_free(m_app);
        m_app = NULL;
        return -1;
    }

    if (gd_app_cfg_reload(m_app) != 0) {
        APP_CTX_ERROR(m_app, "AppContext: load cfg fail!");
        gd_app_context_free(m_app);
        m_app = NULL;
        return -1;
    }

    if (gd_app_modules_load(m_app) != 0) {
        gd_app_context_free(m_app);
        APP_CTX_ERROR(m_app, "AppContext: create app load module fail!");
        m_app = NULL;
        return -1;
    }

    APP_CTX_INFO(m_app, "AppContext: create app load module success!");

    /*获取Manip类 */
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    jobject activity_cls = env->FindClass("com/drowgames/helper/DrowActivity");
    if (activity_cls == NULL) {
        gd_app_context_free(m_app);
        APP_CTX_ERROR(m_app, "AppContext_init: get DrowActivity class fail!");
        m_app = NULL;
        return -1;
    }
    
	m_activity_cls = (jclass)env->NewGlobalRef(activity_cls);
    if (m_activity_cls == NULL) {
        gd_app_context_free(m_app);
        APP_CTX_ERROR(m_app, "AppContext_init: NewGlobalRef DrowActivity class fail!");
        m_app = NULL;
        return -1;
    }

    m_render_commit_begin = env->GetMethodID(m_activity_cls, "render_commit_begin", "()Z");
    assert(m_render_commit_begin);
    m_render_commit_done = env->GetMethodID(m_activity_cls, "render_commit_done", "()V");
    assert(m_render_commit_done);
    
    return 0;
}

void AppContext::fini(void) {
 	JNIEnv *env = (JNIEnv *)android_jni_env();
    
    m_render_commit_begin = NULL;
    m_render_commit_done = NULL;
    env->DeleteGlobalRef(m_activity_cls);
    
    assert(m_app);
    gd_app_context_free(m_app);
    m_app = NULL;
}

AppContext * AppContext::s_ins = NULL;

}}

