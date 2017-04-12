#ifndef APPSVR_PAYMENT_APPLE_PURCHASE_IOS_DELEGATE_H
#define APPSVR_PAYMENT_APPLE_PURCHASE_IOS_DELEGATE_H
#include "include/TGSDK/TGSDK.h"
#include "../appsvr_yomobads_module_i.h"

@interface YomobadsDelegate : NSObject<TGPreloadADDelegate,TGRewardVideoADDelegate>
    - (id)initWithModule:(appsvr_yomobads_module_t)adapter;
    - (void)showAd:(NSString *)type;
@end

#endif
