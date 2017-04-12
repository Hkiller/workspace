#include <cassert>
#include <stdexcept>
#include "gd/app/app_log.h"
#include "cpepp/dr/Utils.hpp"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_worker.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_phase.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "protocol/plugin/app_env/app_env_pro.h"
#include "AppContext.hpp"
#include "../../../EnvExt.hpp"
#include "../../../UICenterExt.hpp"
#include "../../../RuningExt.hpp"

using namespace UI::App;

extern "C" {

int android_asset_set_mgr(JNIEnv* env, jobject assetManager);
    
JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_setJavaVM(JNIEnv* env, jclass cls) {
    JavaVM *jvm;
    env->GetJavaVM(&jvm);
    android_set_jvm(jvm);
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_setAndroidApk(JNIEnv* env, jclass cls, jstring apk) {
    char * szApk = (char*)env->GetStringUTFChars(apk, NULL);
    android_set_current_apk(szApk);
    env->ReleaseStringUTFChars(apk, szApk);
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_setInternalDir(JNIEnv* env, jclass cls, jstring path) {
    char * szPath = (char*)env->GetStringUTFChars(path, NULL);
    android_set_internal_dir(szPath);
    env->ReleaseStringUTFChars(path, szPath);
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_setExternalDir(JNIEnv* env, jclass cls, jstring path) {
    char * szPath = (char*)env->GetStringUTFChars(path, NULL);
    android_set_external_dir(szPath);
    env->ReleaseStringUTFChars(path, szPath);
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_setAssetManager(JNIEnv* env, jclass cls, jobject assetManager) {
    android_asset_set_mgr(env, assetManager);
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeInit(JNIEnv * jni_env, jclass cls, jobject activity) {
    assert(Ui::App::AppContext::s_ins == NULL);

    Ui::App::AppContext::s_ins = new Ui::App::AppContext();

    if (Ui::App::AppContext::s_ins->init(activity) != 0) {
        delete Ui::App::AppContext::s_ins;
        Ui::App::AppContext::s_ins = NULL;
        throw ::std::runtime_error("app context init");
    }

    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    env.runing().init();
}

JNIEXPORT jboolean JNICALL Java_com_drowgames_helper_DrowActivity_nativeIsInitComplete(JNIEnv * jni_env, jclass cls) {
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());

    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(ui_env);

    if (cur_phase == NULL) return JNI_FALSE;

    if (plugin_ui_phase_node_prev(cur_phase) != NULL) return JNI_TRUE;

    switch(plugin_ui_phase_node_state(cur_phase)) {
    case plugin_ui_phase_node_state_processing:
    case plugin_ui_phase_node_state_back:
        return JNI_TRUE;
    default:
        return JNI_FALSE;
    }
}
    
JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeResize(JNIEnv * jni_env, jclass cls, jint w, jint h) {
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());

    ui_vector_2_t base_size = plugin_ui_env_origin_sz(env.uiCenter().uiEnv());
    if ((base_size->x > base_size->y && w < h)
        || (base_size->x < base_size->y && w > h))
    {
        int32_t t = w; w = h ; h = t;
    }

    env.runing().setSize(w, h);
}


JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnTouch(JNIEnv * jni_env, jclass cls, jint action, jint index, jint x, jint y) {
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    env.runing().processInput((RuningExt::TouchAction)action, index, x, y);
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnKeyBack(JNIEnv * jni_env, jclass cls) {
    try {
        EnvExt::instance(Gd::App::Application::instance()).runing().stop();
    }
    catch(...) {
    }
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnActivityResult(
    JNIEnv * jni_env, jclass cls, jobject intent, jint requestCode, jint resultCode)
{
    plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(Gd::App::Application::instance(), NULL);
    plugin_app_env_android_dispatch_activity_result(app_env, intent, requestCode, resultCode);
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnNewIntent(JNIEnv * jni_env, jclass cls, jobject intent) {
    plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(Gd::App::Application::instance(), NULL);
    plugin_app_env_android_dispatch_new_intent(app_env, intent);
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnKeyMenu(JNIEnv * jni_env, jclass cls) {
	// TODO deal with the keydown event;
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnPause(JNIEnv * jni_env, jclass cls) {
    try {
        EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
        env.runing().setState(ui_runtime_pause);
    }
    catch(...) {
    }
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnStop(JNIEnv * jni_env, jclass cls) {
    try {
        EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
        env.runing().setState(ui_runtime_stop);
        
        // ui_runtime_render_t ctx = env.context();
        // ui_runtime_render_worker_t worker = ui_runtime_render_worker_get(ctx);
        // if (worker) {
        //     ui_runtime_render_worker_free(worker);
        // }
    }
    catch(...) {
    }
}
   
JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnStart(JNIEnv * jni_env, jclass cls) {
    try {
        // plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(Gd::App::Application::instance(), NULL);
        // plugin_app_env_android_dispatch_start(app_env);
    }
    catch(...) {
    }
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnRestart(JNIEnv * jni_env, jclass cls) {
    try {
        // plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(Gd::App::Application::instance(), NULL);
        // plugin_app_env_android_dispatch_start(app_env);
    }
    catch(...) {
    }
}

// static int app_gl_worker_begin_render(void * ctx) {
//  	JNIEnv *env = (JNIEnv *)android_jni_env();

//     assert(Ui::App::AppContext::s_ins);

//     plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(Gd::App::Application::instance(), NULL);
//     jobject context = (jobject)plugin_app_env_android_activity(app_env);
    
//     return env->CallBooleanMethod(
//         context,
//         Ui::App::AppContext::s_ins->m_render_commit_begin) ? 0 : -1;
// }
    
// static void app_gl_worker_end_render(void * ctx) {
//  	JNIEnv *env = (JNIEnv *)android_jni_env();

//     assert(Ui::App::AppContext::s_ins);

//     plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(Gd::App::Application::instance(), NULL);
//     jobject context = (jobject)plugin_app_env_android_activity(app_env);
    
//     env->CallVoidMethod(
//         context,
//         Ui::App::AppContext::s_ins->m_render_commit_done);
// }
    
JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeOnResume(JNIEnv * jni_env, jclass cls) {
    try {
        EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
        env.runing().setState(ui_runtime_runing);

        // ui_runtime_render_t ctx = env.context();
        // ui_runtime_render_worker_create(ctx, NULL, app_gl_worker_begin_render, app_gl_worker_end_render, 1);
    }
    catch(...) {
    }
}

JNIEXPORT void JNICALL Java_com_drowgames_helper_DrowActivity_nativeDone(JNIEnv *env, jobject obj) {
    assert(Ui::App::AppContext::s_ins);
    Ui::App::AppContext::s_ins->fini();
    
    delete Ui::App::AppContext::s_ins;
    Ui::App::AppContext::s_ins = NULL;
}

JNIEXPORT jboolean JNICALL Java_com_drowgames_helper_DrowActivity_nativeRender(JNIEnv * jni_env, jclass cls) {
    gd_app_context_t app = gd_app_ins();
    if (app == NULL) JNI_FALSE;
    
    EnvExt & env = EnvExt::instance(Gd::App::Application::_cast(app));
    ui_runtime_render_t render = env.context();
    // ui_runtime_render_worker_t worker = ui_runtime_render_worker(render);
    // if (worker == NULL) return JNI_FALSE;

    env.runing().update();

    uint8_t have_pre_frame;
    ui_runtime_render_begin(render, &have_pre_frame);
    
    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();
    plugin_ui_env_render(ui_env, render);

    ui_runtime_render_done(render);

    return have_pre_frame ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL Java_com_drowgames_helper_DrowActivity_nativeFixFrameTime(JNIEnv * jni_env, jclass cls) {
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();
    plugin_ui_phase_node_t phase_node = plugin_ui_phase_node_current(ui_env);
    plugin_ui_phase_t phase = phase_node ? plugin_ui_phase_node_current_phase(phase_node) : NULL;
    uint8_t fps = phase ? plugin_ui_phase_fps(phase) : 60;
    return 1000 / fps;
}

JNIEXPORT jboolean JNICALL Java_com_drowgames_helper_DrowActivity_nativeIsStop(JNIEnv * jni_env, jclass cls) {
    gd_app_context_t app = gd_app_ins();

    return app == NULL || gd_app_state(app) == gd_app_done
        ? JNI_TRUE
        : JNI_FALSE;
}
    
}
