#ifndef APPSVR_PAYMENT_IAPPPAY_IOS_DELEGATE_H
#define APPSVR_PAYMENT_IAPPPAY_IOS_DELEGATE_H
#include <IapppayKit/IapppayKit.h>
#include "../appsvr_iapppay_module_i.h"

@interface AppSvrPaymentIAppPayRetDelegate : NSObject <IapppayKitPayRetDelegate>
- (id)initWithModule:(appsvr_iapppay_module_t)adapter;
@end

#endif

