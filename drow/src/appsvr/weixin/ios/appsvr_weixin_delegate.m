#include "cpe/pal/pal_stdlib.h"
#import "appsvr_weixin_delegate.h"

@interface appsvr_weixin_delegate () {
    appsvr_weixin_module_t m_module;
}
@end

@implementation appsvr_weixin_delegate

- (id)initWithModule:(appsvr_weixin_module_t)module
{
    m_module = module;
    return self;
}

- (void)onResp:(BaseResp *)resp {
    if ([resp isKindOfClass:[SendMessageToWXResp class]]) {
        //SendMessageToWXResp *messageResp = (SendMessageToWXResp *)resp;
    }
    else if ([resp isKindOfClass:[SendAuthResp class]]) {
        SendAuthResp *authResp = (SendAuthResp *)resp;

        appsvr_weixin_notify_login_result(
            m_module,
            authResp.code.UTF8String,
            atoi(authResp.state.UTF8String),
            0,
            NULL);        
    }
    else if ([resp isKindOfClass:[AddCardToWXCardPackageResp class]]) {
        //AddCardToWXCardPackageResp *addCardResp = (AddCardToWXCardPackageResp *)resp;
    }
}

- (void)onReq:(BaseReq *)req {
    if ([req isKindOfClass:[GetMessageFromWXReq class]]) {
        //GetMessageFromWXReq *getMessageReq = (GetMessageFromWXReq *)req;
    } else if ([req isKindOfClass:[ShowMessageFromWXReq class]]) {
        //ShowMessageFromWXReq *showMessageReq = (ShowMessageFromWXReq *)req;
    } else if ([req isKindOfClass:[LaunchFromWXReq class]]) {
        //LaunchFromWXReq *launchReq = (LaunchFromWXReq *)req;
    }
}

@end
