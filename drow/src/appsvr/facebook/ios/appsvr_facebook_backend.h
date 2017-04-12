#ifndef APPSVR_STATISTICS_FACEBOOK_IOS_H
#define APPSVR_STATISTICS_FACEBOOK_IOS_H
#import <FBSDKCoreKit/FBSDKCoreKit.h>
#import <FBSDKLoginKit/FBSDKLoginKit.h>
#include "../appsvr_facebook_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_facebook_backend {
    appsvr_facebook_module_t m_module;
    FBSDKLoginManager * m_login_mgr;
};

#ifdef __cplusplus
}
#endif
    
#endif
