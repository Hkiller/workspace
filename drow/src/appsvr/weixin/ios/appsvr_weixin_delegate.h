#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "WXApi.h"
#import "../appsvr_weixin_module_i.h"

@interface appsvr_weixin_delegate : NSObject<WXApiDelegate>
- (id)initWithModule:(appsvr_weixin_module_t)module;
@end
