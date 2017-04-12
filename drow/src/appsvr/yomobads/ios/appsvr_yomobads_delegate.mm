#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_yomobads_delegate.h"
#include "appsvr/ad/appsvr_ad_module.h"
#include "appsvr/ad/appsvr_ad_request.h"



@interface YomobadsDelegate () {
    appsvr_yomobads_module_t m_yomobads;
}
@end

@implementation YomobadsDelegate

- (id)initWithModule:(appsvr_yomobads_module_t)yomobads
{
    m_yomobads = yomobads;
    
    return self;
}

//显示广告
- (void)showAd:(NSString *)type{
    [TGSDK showAd:type];
}

// 广告预加载调用成功
- (void) onPreloadSuccess:(NSString*)result{
}

// 广告预加载调用失败
- (void) onPreloadFailed:(NSString*)result WithError:(NSString*) error{
}

// 静态插屏广告已就绪
- (void) onCPADLoaded:(NSString *)result{
}

// 视频广告已就绪
- (void) onVideoADLoaded:(NSString *)result{
}

// 广告开始播放
- (void) onShowSuccess:(NSString*)result{
}

// 广告播放失败
- (void) onShowFailed:(NSString*)result WithError:(NSString*) error{
    appsvr_ad_request_set_result(appsvr_ad_request_find_by_id(m_yomobads->m_ad_module,m_yomobads->m_request_id),appsvr_ad_start_fail);
}

// 广告播放完成
- (void) onADComplete:(NSString*)result {
}

// 用户点击了广告，正在跳转到其他页面
- (void) onADClick:(NSString*)result{
}

// 广告关闭
- (void) onADClose:(NSString*)result{
}

// 奖励广告条件达成，可以向用户发放奖励
- (void) onADAwardSuccess:(NSString*)result{
    appsvr_ad_request_set_result(appsvr_ad_request_find_by_id(m_yomobads->m_ad_module,m_yomobads->m_request_id),appsvr_ad_start_success);
}

// 奖励广告条件未达成，无法向用户发放奖励
- (void) onADAwardFailed:(NSString*)result WithError:(NSError *)error{
}

@end
