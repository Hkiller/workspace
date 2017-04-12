#ifndef PLUGIN_EDITOR_BACKEND_I_H
#define PLUGIN_EDITOR_BACKEND_I_H
#include <jni.h>
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../plugin_editor_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_editor_backend {
    plugin_app_env_module_t m_app_env;

    /*jni*/
    jobject m_listener;
    jmethodID m_listener_update_context;
    jmethodID m_listener_update_selection;
    jmethodID m_listener_close_keyboard;
    jmethodID m_listener_copy_to_clipboard;
    jmethodID m_listener_get_from_clipboard;
};

#ifdef __cplusplus
}
#endif

#endif
