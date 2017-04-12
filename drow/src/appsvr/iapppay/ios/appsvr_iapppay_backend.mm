#include <IapppayKit/IapppayOrderUtils.h>
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_iapppay_delegate.h"
#include "../appsvr_iapppay_payment_adapter_i.h"

struct appsvr_iapppay_backend {
    AppSvrPaymentIAppPayRetDelegate * m_delegate;
};

int appsvr_iapppay_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req) {
    appsvr_iapppay_payment_adapter_t payment_adapter = (appsvr_iapppay_payment_adapter_t)appsvr_payment_adapter_data(adapter);
    appsvr_iapppay_module_t iapppay = payment_adapter->m_module;

    IapppayOrderUtils *orderInfo = [[IapppayOrderUtils alloc] init];
    orderInfo.appId = [NSString stringWithUTF8String: iapppay->m_app_id];
    orderInfo.cpPrivateKey = [NSString stringWithUTF8String: iapppay->m_appv_key];
    orderInfo.notifyUrl = [NSString stringWithUTF8String: req->notify_to];
    orderInfo.cpOrderId = [NSString stringWithUTF8String: req->trade_id];

    if (iapppay->m_free_pay_product_id) {
        orderInfo.waresId = [NSString stringWithUTF8String: iapppay->m_free_pay_product_id];
    }
    else {
        char product_buf[32];
        snprintf(product_buf, sizeof(product_buf), "%d", req->product_id);
        orderInfo.waresId = [NSString stringWithUTF8String: product_buf];
    }
    orderInfo.waresName = [NSString stringWithUTF8String: req->product_name];

    char price_buf[32];
    snprintf(price_buf, sizeof(price_buf), "%.2f", req->price);
    orderInfo.price = [NSString stringWithUTF8String: price_buf];

    orderInfo.appUserId = [NSString stringWithUTF8String: req->user_id];

    NSString *trandInfo = [orderInfo getTrandData];
    int rv = [[IapppayKit sharedInstance] makePayForTrandInfo: trandInfo
                                                  payDelegate: iapppay->m_backend->m_delegate];
    if (rv != IAPPPAY_PAYRETCODE_SUCCESS) {
        CPE_ERROR(iapppay->m_em, "iapppay: start trade fail, rv=%d", rv);
        return -1;
    }
    
    return 0;
}

int appsvr_iapppay_backend_init(appsvr_iapppay_module_t iapppay) {
    cfg_t global_cfg = cfg_find_cfg(gd_app_cfg(iapppay->m_app), "args");
    cfg_t iapppay_cfg = cfg_find_cfg(global_cfg, "iapppay");
    cfg_t ios_cfg = cfg_find_cfg(iapppay_cfg, "ios");

    if (appsvr_iapppay_module_set_chanel(iapppay, cfg_get_string(global_cfg, "chanel", NULL)) != 0
        || appsvr_iapppay_module_set_url(iapppay, cfg_get_string(global_cfg, "url-prefix", NULL)) != 0
        || appsvr_iapppay_module_set_free_pay_product_id(iapppay, cfg_get_string(iapppay_cfg, "free-pay-product-id", NULL)) != 0
        || appsvr_iapppay_module_set_app_id(iapppay, cfg_get_string(ios_cfg, "app-id", NULL)) != 0
        || appsvr_iapppay_module_set_appv_key(iapppay, cfg_get_string(ios_cfg, "appv-key", NULL)) != 0
        || appsvr_iapppay_module_set_platp_key(iapppay, cfg_get_string(ios_cfg, "platp-key", NULL)) != 0
        )
    {
        CPE_ERROR(iapppay->m_em, "appsvr_iapppay: set config data fail!");
        return -1;
    }

    [[IapppayKit sharedInstance] setAppId: [NSString stringWithUTF8String: iapppay->m_app_id]
                                    mACID: iapppay->m_chanel ? [NSString stringWithUTF8String: iapppay->m_chanel] : nil];

    [[IapppayKit sharedInstance] setAppAlipayScheme: [NSString stringWithUTF8String: iapppay->m_url]];

    iapppay->m_backend
        = (appsvr_iapppay_backend_t)mem_alloc(iapppay->m_alloc, sizeof(struct appsvr_iapppay_backend));
    if (iapppay->m_backend == NULL) {
        CPE_ERROR(iapppay->m_em, "appsvr_iapppay: alloc backend fail!");
        return -1;
    }
    iapppay->m_backend->m_delegate = [[AppSvrPaymentIAppPayRetDelegate alloc] initWithModule: iapppay];

    return 0;
}

void appsvr_iapppay_backend_fini(appsvr_iapppay_module_t iapppay) {
    assert(iapppay->m_backend);

    iapppay->m_backend->m_delegate = nil;
    
    mem_free(iapppay->m_alloc, iapppay->m_backend);
    iapppay->m_backend = NULL;
}

