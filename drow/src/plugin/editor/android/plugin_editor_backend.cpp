#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "plugin/layout/plugin_layout_render.h"
#include "plugin/layout/plugin_layout_animation_selection.h"
#include "plugin/layout/plugin_layout_animation_caret.h"
#include "plugin_editor_backend_i.hpp"
#include "../plugin_editor_editing_i.h"

int plugin_editor_backend_init(plugin_editor_module_t module) {
 	JNIEnv *env = android_jni_env();
    plugin_editor_backend_t backend;

    backend = (plugin_editor_backend_t)mem_alloc(module->m_alloc, sizeof(plugin_editor_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "plugin_editor_backend: init: alloc fail!");
        return -1; 
    }

    backend->m_app_env = plugin_app_env_module_find_nc(module->m_app, NULL);
    if (backend->m_app_env == NULL) {
        CPE_ERROR(module->m_em, "plugin_editor_backend: init: no app env!");
        mem_free(module->m_alloc, backend->m_app_env);
        return -1;
    }

    /*获取Manip类 */
    jclass listener_cls = (jclass)env->FindClass("com/drowgames/editor/TextInputWraper");
    if (listener_cls == NULL) {
        CPE_ERROR(module->m_em, "plugin_editor_backend: init: get TextInputWraper class fail!");
        return -1;
    }

    jmethodID init = env->GetMethodID(listener_cls, "<init>", "(JLandroid/app/Activity;)V");
    assert(init);
    
    jobject listener =
        env->NewObject(
            listener_cls, init, (jlong)module, (jobject)plugin_app_env_android_activity(backend->m_app_env));
    if (!plugin_app_env_android_check_exception(module->m_em) || listener == NULL) {
        CPE_ERROR(module->m_em, "plugin_editor_backend: init: create listener fail!");
        return -1;
    }

    backend->m_listener = env->NewGlobalRef(listener);
    if (!plugin_app_env_android_check_exception(module->m_em) || backend->m_listener == NULL) {
        CPE_ERROR(module->m_em, "plugin_editor_backend: init: dup listener fail!");
        return -1;
    }

    backend->m_listener_update_context = env->GetMethodID(listener_cls, "updateContext", "(Ljava/lang/String;III)V");
    assert(backend->m_listener_update_context);

    backend->m_listener_update_selection = env->GetMethodID(listener_cls, "updateSelection", "(II)V");
    assert(backend->m_listener_update_selection);
    
    backend->m_listener_close_keyboard = env->GetMethodID(listener_cls, "closeKeyBoard", "()V");
    assert(backend->m_listener_close_keyboard);

    backend->m_listener_get_from_clipboard = env->GetMethodID(listener_cls, "getFromClipboard", "()Ljava/lang/String;");
    assert(backend->m_listener_get_from_clipboard);

    backend->m_listener_copy_to_clipboard = env->GetMethodID(listener_cls, "copyToClipboard", "(Ljava/lang/String;)V");
    assert(backend->m_listener_copy_to_clipboard);
    
    module->m_backend = backend;
    
    return 0;
}

void plugin_editor_backend_fini(plugin_editor_module_t module) {
    plugin_editor_backend_t backend = module->m_backend;
 	JNIEnv *env = android_jni_env();
    
    assert(backend);

    env->DeleteGlobalRef(backend->m_listener);
    
    mem_free(module->m_alloc, module->m_backend);
    module->m_backend = NULL;
}

void plugin_editor_backend_update_content(plugin_editor_module_t module, plugin_layout_render_t render) {
    plugin_editor_backend_t backend = module->m_backend;
 	JNIEnv *env = android_jni_env();

    char * text_utf8 = plugin_layout_render_text_utf8(module->m_alloc, render);
    if (text_utf8 == NULL) {
        CPE_ERROR(module->m_em, "plugin_editor_backend_update: get render text fail");
        return;
    }
    
    env->CallVoidMethod(
        backend->m_listener,
        backend->m_listener_update_context,
        env->NewStringUTF(text_utf8),
        (jint)module->m_active_editing->m_is_passwd,
        (jint)module->m_active_editing->m_number_only,
        (jint)module->m_active_editing->m_max_len);
    if (!plugin_app_env_android_check_exception(module->m_em)) {
        CPE_ERROR(module->m_em, "plugin_editor_backend_update_content: call jni fun fail!");
    }

    mem_free(module->m_alloc, text_utf8);
}

void plugin_editor_backend_update_selection(plugin_editor_module_t module, plugin_layout_render_t render) {
    plugin_editor_backend_t backend = module->m_backend;
 	JNIEnv *env = android_jni_env();
    plugin_layout_animation_selection_t selection;
    plugin_layout_animation_caret_t caret;

    if ((selection = plugin_layout_animation_selection_find_first(render))) {
        env->CallVoidMethod(
            backend->m_listener,
            backend->m_listener_update_selection,
            (jint)plugin_layout_animation_selection_begin_pos(selection),
            (jint)plugin_layout_animation_selection_end_pos(selection));
        if (!plugin_app_env_android_check_exception(module->m_em)) {
            CPE_ERROR(module->m_em, "plugin_editor_backend_update_selection: call jni fun fail!");
        }
    }
    else if ((caret = plugin_layout_animation_caret_find_first(render))) {
        env->CallVoidMethod(
            backend->m_listener,
            backend->m_listener_update_selection,
            (jint)plugin_layout_animation_caret_pos(caret),
            (jint)plugin_layout_animation_caret_pos(caret));
        if (!plugin_app_env_android_check_exception(module->m_em)) {
            CPE_ERROR(module->m_em, "plugin_editor_backend_update_selection: call jni fun fail!");
        }
    }
}

char * plugin_editor_backend_clipboard_get(plugin_editor_module_t module) {
    plugin_editor_backend_t backend = module->m_backend;
 	JNIEnv *env = android_jni_env();

    jstring j_str = (jstring)env->CallObjectMethod(backend->m_listener, backend->m_listener_get_from_clipboard);
    if (!plugin_app_env_android_check_exception(module->m_em)) {
        CPE_ERROR(module->m_em, "plugin_editor_backend_clipboard_get: call jni fun fail!");
        return NULL;
    }

    if (j_str == NULL) return NULL;

    const char * c_str = env->GetStringUTFChars(j_str, NULL);
    if (c_str == NULL) {
        CPE_ERROR(module->m_em, "plugin_editor_backend_clipboard_get: jni GetStringUTFChars fail!");
        return NULL;
    }
    
    char * r = cpe_str_mem_dup(module->m_alloc, c_str);

    env->ReleaseStringUTFChars(j_str, c_str);

    return r;
}

int plugin_editor_backend_clipboard_put(plugin_editor_module_t module, const char * data) {
    plugin_editor_backend_t backend = module->m_backend;
 	JNIEnv *env = android_jni_env();

    env->CallVoidMethod(
        backend->m_listener,
        backend->m_listener_copy_to_clipboard,
        env->NewStringUTF(data));
    if (!plugin_app_env_android_check_exception(module->m_em)) {
        CPE_ERROR(module->m_em, "plugin_editor_backend_clipboard_put: call jni fun fail!");
        return -1;
    }

    return 0;
}

void plugin_editor_backend_close(plugin_editor_module_t module) {
	JNIEnv *env = android_jni_env();
    
    env->CallVoidMethod(
        module->m_backend->m_listener,
        module->m_backend->m_listener_close_keyboard);
    if (!plugin_app_env_android_check_exception(module->m_em)) {
        CPE_ERROR(module->m_em, "plugin_editor_backend_close: call jni fun fail!");
    }
}

extern "C" {

JNIEXPORT void JNICALL Java_com_drowgames_editor_TextInputWraper_nativeCommitContent(JNIEnv * jni_env, jclass cls, jlong ptr, jstring text) {
    plugin_editor_module_t module = (plugin_editor_module_t)(ptr);
	JNIEnv *env = android_jni_env();

    const char * c_text = env->GetStringUTFChars(text, NULL);
    if (c_text == NULL) {
        CPE_ERROR(module->m_em, "plugin_editor_backend: commit text: to utf-8 str fail!");
        return;
    }
    
    plugin_editor_editing_commit_data_utf8(module->m_active_editing, c_text);
    jni_env->ReleaseStringUTFChars(text, c_text);
}

JNIEXPORT void JNICALL Java_com_drowgames_editor_TextInputWraper_nativeCommitSelection(JNIEnv * jni_env, jclass cls, jlong ptr, jint begin_pos, jint end_pos) {
    plugin_editor_module_t module = (plugin_editor_module_t)(ptr);
    plugin_editor_editing_commit_selection(module->m_active_editing, begin_pos, end_pos);
    CPE_ERROR(module->m_em, "plugin_editor_backend: commit selection: to %d,%d", (int)begin_pos, (int)end_pos);
}

JNIEXPORT void JNICALL Java_com_drowgames_editor_TextInputWraper_nativeStop(JNIEnv * jni_env, jclass cls, jlong ptr) {
    plugin_editor_module_t module = (plugin_editor_module_t)(ptr);
    plugin_editor_editing_set_is_active(module->m_active_editing, 0);
}

}
