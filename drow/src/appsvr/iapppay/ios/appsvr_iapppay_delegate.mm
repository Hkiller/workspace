#include <IapppayKit/IapppayOrderUtils.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_iapppay_delegate.h"

@interface AppSvrPaymentIAppPayRetDelegate () {
    appsvr_iapppay_module_t m_iapppay;
}
@end

@implementation AppSvrPaymentIAppPayRetDelegate

- (id)initWithModule:(appsvr_iapppay_module_t)iapppay
{
    m_iapppay = iapppay;
    return self;
}

- (void)iapppayKitRetPayStatusCode:(IapppayKitPayRetCodeType)statusCode
                        resultInfo:(NSDictionary *)resultInfo
{
    APPSVR_PAYMENT_RESULT result;
    bzero(&result, sizeof(result));

    NSString * RetCode = resultInfo[@"RetCode"];
    NSString * ErrorMsg = resultInfo[@"ErrorMsg"];

    if (statusCode == IAPPPAY_PAYRETCODE_SUCCESS) {
        BOOL isSuccess = [IapppayOrderUtils checkPayResult: resultInfo[@"Signature"]
                                                withAppKey: [NSString stringWithUTF8String: m_iapppay->m_platp_key]];
        if (isSuccess) {
            /*支付成功，验签成功 */
            CPE_INFO(m_iapppay->m_em, "iapppay: pay success");

            result.result = appsvr_payment_success;
        } else {
            /*支付成功，验签失败 */
            CPE_ERROR(m_iapppay->m_em, "iapppay: pay success, but verify signature error");

            result.result = appsvr_payment_failed;
        }
    }
    else if (statusCode == IAPPPAY_PAYRETCODE_CANCELED) {
        /*支付取消 */
        CPE_ERROR(
            m_iapppay->m_em, "iapppay: pay cancel, RetCode=%d, ErrorMsg=%s",
            atoi(RetCode.UTF8String), ErrorMsg.UTF8String);
        result.result = appsvr_payment_canceled;
        result.service_result = atoi(RetCode.UTF8String);
        cpe_str_dup(result.error_msg, sizeof(result.error_msg), ErrorMsg.UTF8String);
    }
    else {
        if (statusCode != IAPPPAY_PAYRETCODE_FAILED) {
            CPE_ERROR(
                m_iapppay->m_em, "iapppay: pay unknown status code %d, RetCode=%d, ErrorMsg=%s",
                statusCode, atoi(RetCode.UTF8String), ErrorMsg.UTF8String);
        }
        
        int32_t service_result = atoi(RetCode.UTF8String);

        if (service_result == 6110) {
            result.result = appsvr_payment_already_payed;
        }
        else {
            /*支付失败 */
            CPE_ERROR(
                m_iapppay->m_em, "iapppay: pay fail, RetCode=%d, ErrorMsg=%s",
                atoi(RetCode.UTF8String), ErrorMsg.UTF8String);
            result.result = appsvr_payment_failed;
            result.service_result = service_result;
            cpe_str_dup(result.error_msg, sizeof(result.error_msg), ErrorMsg.UTF8String);
        }
    }

    appsvr_payment_adapter_notify_result(m_iapppay->m_adapter, &result);
}

@end
