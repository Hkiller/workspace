#include <UIKit/UIKit.h>
#include "../EnvExt.hpp"

namespace UI { namespace App {

const char * EnvExt::detectLanguage(void) const {
    const char * aa = m_language.c_str();
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *currentLanguage = [languages objectAtIndex:0];
        
    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:currentLanguage];
    NSString * languageCode = [temp objectForKey:NSLocaleLanguageCode];
        
    if ([languageCode isEqualToString:@"zh"]) {
        return "cn";
    }
    else if ([languageCode isEqualToString:@"en"]) {
        return "en";
    }
    else {
        return "cn";
    }
}

}}

