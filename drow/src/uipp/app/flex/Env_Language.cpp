#include "System.hpp"
#include "../EnvExt.hpp"

namespace UI { namespace App {

const char * EnvExt::detectLanguage(void) const {
    if (m_language.empty()) {
        char * language = internal::utf8_toString(flash::system::Capabilities::language);
        APP_CTX_INFO(m_app, "language: %s", language);

        if (strcmp(language, "en") == 0) {
            m_language = "en";
        }
        else if (strcmp(language, "zh-CN") == 0) {
            m_language = "cn";
        }
        else {
            m_language = "cn";
        }
        
        free(language);
    }
    
    return m_language.c_str();
}

}}

