#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_apple_purchase_delegate.h"
#include "appsvr_payment_product_i.h"

@interface PurchaseDelegate () {
    appsvr_apple_purchase_module_t m_apple_purchase;
}
@end

@implementation PurchaseDelegate

- (id)initWithModule:(appsvr_apple_purchase_module_t)apple_purchase
{
    m_apple_purchase = apple_purchase;

    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];

    return self;
}

- (void)requestProducts:(NSMutableSet *)nsset{
    // "异步"询问苹果能否销售
    //NSArray *product = [[NSArray alloc] init];
    
    SKProductsRequest *request = [[SKProductsRequest alloc] initWithProductIdentifiers:nsset];
    request.delegate = self;
    // 启动请求
    [request start];
}

// //请求商品
- (void)requestProductData:(NSString *)type{
    NSLog(@"-------------请求对应的产品信息----------------");
    NSArray *product = [[NSArray alloc] initWithObjects:type, nil,nil];

    NSSet *nsset = [NSSet setWithArray:product];
    SKProductsRequest *request = [[SKProductsRequest alloc] initWithProductIdentifiers:nsset];
    request.delegate = self;
    [request start];
}


//收到产品返回信息
- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response {
    
    NSLog(@"--------------收到产品反馈消息---------------------");
    NSArray *product = response.products;
    if([product count] == 0){
        NSLog(@"--------------没有商品------------------");
        APPSVR_PAYMENT_RESULT result;
        bzero(&result, sizeof(result));
        result.result = appsvr_payment_failed;
        appsvr_payment_adapter_notify_result(m_apple_purchase->m_payment_adapter, &result);

        return;
    }
    
    //NSLog(@"productID:%@", response.invalidProductIdentifiers);
    //NSLog(@"产品付费数量:%d",[product count]);
    

    if([product count] == 1)
    {
        SKProduct *p = nil;
        for (SKProduct *pro in product) {
            NSLog(@"%@", [pro description]);
            NSLog(@"%@", [pro localizedTitle]);
            NSLog(@"%@", [pro localizedDescription]);
            NSLog(@"%@", [pro price]);
            NSLog(@"%@", [pro productIdentifier]);
            int price = pro.price.integerValue;
            //NSLog(@"%@", price);
            NSString *symbol = [pro.priceLocale objectForKey:NSLocaleCurrencySymbol];
            NSLog(@"%@", symbol);
            p = pro;
        }
        SKPayment *payment = [SKPayment paymentWithProduct:p];
        
        NSLog(@"发送购买请求");
        [[SKPaymentQueue defaultQueue] addPayment:payment];
    }
    else
    {
        SKProduct *p = nil;
        for (SKProduct *pro in product) {
            NSLog(@"%@", [pro description]);
            NSLog(@"%@", [pro localizedTitle]);
            NSLog(@"%@", [pro localizedDescription]);
            NSLog(@"%@", [pro price]);
            NSLog(@"%@", [pro productIdentifier]);
            int price = pro.price.integerValue;
            //NSLog(@"%@", price);
            NSString *symbol = [pro.priceLocale objectForKey:NSLocaleCurrencySymbol];
            NSLog(@"%@", symbol);
            char product_buf[64];
            snprintf(product_buf, sizeof(product_buf), "%d", (int)pro.productIdentifier.intValue);

            char price_buf[64];
            snprintf(price_buf, sizeof(price_buf), "%s%0.2f",[symbol UTF8String],(float)pro.price.floatValue);

            appsvr_payment_product_create(m_apple_purchase->m_payment_module
                                          ,m_apple_purchase->m_payment_adapter
                                          ,product_buf
                                          ,price_buf);
        }
        appsvr_payment_adapter_notify_product_sync_done(m_apple_purchase->m_payment_adapter);

    }
}

//请求失败
- (void)request:(SKRequest *)request didFailWithError:(NSError *)error{
    APPSVR_PAYMENT_RESULT result;
    bzero(&result, sizeof(result));
    result.result = appsvr_payment_failed;
    appsvr_payment_adapter_notify_result(m_apple_purchase->m_payment_adapter, &result);
    NSLog(@"------------------错误-----------------:%@", error);
}

- (void)requestDidFinish:(SKRequest *)request{
    NSLog(@"------------反馈信息结束-----------------");
}


//监听购买结果
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transaction{
    for(SKPaymentTransaction *tran in transaction){
        
        APPSVR_PAYMENT_RESULT result;
        bzero(&result, sizeof(result));

        switch (tran.transactionState) {
            case SKPaymentTransactionStatePurchased:
                NSLog(@"交易完成");
                result.result = appsvr_payment_success;
                appsvr_payment_adapter_notify_result(m_apple_purchase->m_payment_adapter, &result);
                [self completeTransaction:tran];
                break;
            case SKPaymentTransactionStatePurchasing:
                NSLog(@"商品添加进列表");
                break;
            case SKPaymentTransactionStateRestored:
                NSLog(@"已经购买过商品");
                break;
            case SKPaymentTransactionStateFailed:
                NSLog(@"交易失败");
                result.result = appsvr_payment_failed;
                appsvr_payment_adapter_notify_result(m_apple_purchase->m_payment_adapter, &result);
                [self completeTransaction:tran];
                break;
            default:
                break;
        }
        
    }
}

//交易结束
- (void)completeTransaction:(SKPaymentTransaction *)transaction{
    //交易成功
    NSLog(@"交易结束");
    
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
}


- (void)dealloc{
    [[SKPaymentQueue defaultQueue] removeTransactionObserver:self];
    [super dealloc];
}

@end
