#ifndef APPSVR_PAYMENT_APPLE_PURCHASE_IOS_DELEGATE_H
#define APPSVR_PAYMENT_APPLE_PURCHASE_IOS_DELEGATE_H
#include <StoreKit/StoreKit.h>
#include "../appsvr_apple_purchase_module_i.h"

@interface PurchaseDelegate : NSObject<SKPaymentTransactionObserver,SKProductsRequestDelegate>
 - (id)initWithModule:(appsvr_apple_purchase_module_t)adapter;
@end

#endif
