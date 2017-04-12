#include <assert.h>
#import <FBSDKShareKit/FBSDKShareKit.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "plugin/app_env/ios/plugin_app_env_ios.h"
#include "appsvr/share/appsvr_share_request.h"
#include "appsvr/share/appsvr_share_request_block.h"
#include "../appsvr_facebook_share_module_i.h"

int appsvr_facebook_share_backend_init(appsvr_facebook_share_module_t module) {
    return 0;
}

void appsvr_facebook_share_backend_fini(appsvr_facebook_share_module_t module) {
}

int appsvr_facebook_share_backend_commit(appsvr_facebook_share_module_t module, appsvr_share_request_t req) {
    appsvr_facebook_share_backend_t backend = module->m_backend;

    @try {
        UIApplication * application = (UIApplication *) plugin_app_env_ios_application(module->m_app_env);
        UIViewController * viewController = application.delegate.window.rootViewController;

        FBSDKShareLinkContent *content = [[FBSDKShareLinkContent alloc] init];
        const char * contentURL =appsvr_share_request_block_get_str(req, appsvr_share_request_block_navigation, 0, "");
        content.contentURL = [NSURL URLWithString: [NSString  stringWithUTF8String: appsvr_share_request_block_get_str(req, appsvr_share_request_block_navigation, 0, "")]];
        
        const char * contentTitle =appsvr_share_request_block_get_str(req, appsvr_share_request_block_title, 0, "");
        content.contentTitle = [NSString  stringWithUTF8String: appsvr_share_request_block_get_str(req, appsvr_share_request_block_title, 0, "")];
       
        const char * imageURL =appsvr_share_request_block_get_str(req, appsvr_share_request_block_remote_picture, 0, "");
        content.imageURL = [NSURL URLWithString: [NSString  stringWithUTF8String: appsvr_share_request_block_get_str(req, appsvr_share_request_block_remote_picture, 0, "")]];
        
        [FBSDKShareDialog showFromViewController:viewController.parentViewController
                                        withContent:content
                                        delegate:nil];

        appsvr_share_request_set_done(req, 1);

        return 0;
    }
    @catch(NSException * e) {
        CPE_ERROR(module->m_em, "appsvr_facebook_share_backend_commit: catch exceptin %s: reasion=%s",
                    [e.name UTF8String],
                    [e.reason UTF8String]);
        appsvr_share_request_set_done(req, 0);
        return -1;
    }
}

