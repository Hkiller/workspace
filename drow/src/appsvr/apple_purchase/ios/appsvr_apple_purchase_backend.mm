#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_apple_purchase_delegate.h"

struct appsvr_apple_purchase_backend {
    PurchaseDelegate * m_delegate;
};

int appsvr_apple_purchase_backend_pay_start(appsvr_apple_purchase_module_t apple_purchase, APPSVR_PAYMENT_BUY const * req) {
    char product_buf[64];
    snprintf(product_buf, sizeof(product_buf), "%d", req->chanel_pay_id);
    NSString *product = [NSString stringWithUTF8String: product_buf];

    if(![SKPaymentQueue canMakePayments]) {
        CPE_ERROR(apple_purchase->m_em, "apple_purchase: check canMakePayments fail fail");
        return -1;
    }

    [apple_purchase->m_backend->m_delegate requestProductData: product];
    

    return 0;
}

int appsvr_apple_purchase_backend_do_sync_products(appsvr_apple_purchase_module_t module)
{
    NSMutableSet *nsset = [[NSMutableSet alloc ]init];
    [nsset addObject: @"1"];
    [nsset addObject: @"2"];
    [nsset addObject: @"3"];
    [nsset addObject: @"4"];
    [nsset addObject: @"5"];
    [nsset addObject: @"6"];
    [nsset addObject: @"7"];
    [nsset addObject: @"8"];
    [nsset addObject: @"9"];
    
    [module->m_backend->m_delegate requestProducts: nsset];
    return 0;
}

// - (IBAction)purchaseFunc:(id)sender {
//     NSString *product = self.productID.text;
//     if([SKPaymentQueue canMakePayments]){
//         [self requestProductData:product];
//     }else{
//         NSLog(@"不允许程序内付费");
//     }
// }


int appsvr_apple_purchase_backend_init(appsvr_apple_purchase_module_t apple_purchase) {
    // cfg_t global_cfg = cfg_find_cfg(gd_app_cfg(apple_purchase->m_app), "args");
    // cfg_t apple_purchase_cfg = cfg_find_cfg(global_cfg, "apple_purchase");
    // cfg_t ios_cfg = cfg_find_cfg(apple_purchase_cfg, "ios");

    // [[apple_purchaseKit sharedInstance] setAppId: [NSString stringWithUTF8String: apple_purchase->m_app_id]
    //                                 mACID: apple_purchase->m_chanel ? [NSString stringWithUTF8String: apple_purchase->m_chanel] : nil];

    // [[apple_purchaseKit sharedInstance] setAppAlipayScheme: [NSString stringWithUTF8String: apple_purchase->m_url]];

     apple_purchase->m_backend
         = (appsvr_apple_purchase_backend_t)mem_alloc(apple_purchase->m_alloc, sizeof(struct appsvr_apple_purchase_backend));
     if (apple_purchase->m_backend == NULL) {
         CPE_ERROR(apple_purchase->m_em, "appsvr_apple_purchase: alloc backend fail!");
         return -1;
     }
     apple_purchase->m_backend->m_delegate = [[PurchaseDelegate alloc] initWithModule: apple_purchase];

     return 0;
}

void appsvr_apple_purchase_backend_fini(appsvr_apple_purchase_module_t apple_purchase) {
    assert(apple_purchase->m_backend);

    apple_purchase->m_backend->m_delegate = nil;
    
    mem_free(apple_purchase->m_alloc, apple_purchase->m_backend);
    apple_purchase->m_backend = NULL;
}

