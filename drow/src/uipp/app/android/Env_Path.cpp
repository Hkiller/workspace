#include "../EnvExt.hpp"
#include "plugin/app_env/android/plugin_app_env_android.hpp"

namespace UI { namespace App {

const char * EnvExt::documentPath(void) const {
    if (!m_documentPath.empty()) return m_documentPath.c_str();
    m_documentPath = android_internal_dir();
    return m_documentPath.c_str();
}

}}
