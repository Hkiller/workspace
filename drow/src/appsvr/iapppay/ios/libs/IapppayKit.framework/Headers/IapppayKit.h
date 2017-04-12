//
//  IapppayKit.h
//  IapppayDemo
//
//  Created by Shixiong on 15/5/8.
//  Copyright (c) 2015年 爱贝. All rights reserved.
//

////////////////////////////////////////////////////////
///////////version：4.0.1-2.1 date：2015.11.10///////////
////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

// 爱贝支付结果Code
typedef enum IapppayKitPayRetCodeType : NSInteger {
    IAPPPAY_PAYRETCODE_CANCELED = 99,    //支付取消
    IAPPPAY_PAYRETCODE_FAILED   = -1,    //支付失败
    IAPPPAY_PAYRETCODE_SUCCESS  = 0      //支付成功
} IapppayKitPayRetCodeType;

@protocol IapppayKitPayRetDelegate <NSObject>

@optional
/**
 * 爱贝支付结果返回
 * @statusCode = "支付结果code"
 * @resultInfo = {RetCode:"错误码",ErrorMsg:"错误信息",Signature:"支付成功的密文，需要验签"}
 **/
- (void)iapppayKitRetPayStatusCode:(IapppayKitPayRetCodeType)statusCode
                        resultInfo:(NSDictionary *)resultInfo;

@end


@interface IapppayKit : NSObject

/**
 * 设置支付宝支付回调
 * @params:appAlipayScheme 支付宝支付SSO回调，该字段必传。
 **/
@property (nonatomic, copy) NSString *appAlipayScheme;


/**
 * 创建SDK接入对象
 **/
+ (IapppayKit *)sharedInstance;


/**
 * 支付宝App支付结果回调
 **/
- (void)handleOpenUrl:(NSURL *)url;


/**
 * 设置爱贝支付窗口的方向(支持横屏和竖屏)
 * 横屏：UIInterfaceOrientationMaskLandscape
 *      UIInterfaceOrientationMaskLandscapeLeft
 *      UIInterfaceOrientationMaskLandscapeRight
 * 竖屏：UIInterfaceOrientationMaskPortrait
 *
 * 注意：
 *      设置的方向需要在app的info.plist中存在，否则导致设置方向无效，也可能会导致崩溃。
 *      只设置支持横屏的应用，需要在AppDelegate中实现application:supportedInterfaceOrientationsForWindow:方法
 *      可不设置支付方向，会根据app的info.plist自动适应
 **/
- (void)setIapppayPayWindowOrientationMask:(UIInterfaceOrientationMask)orientationMask;


/**
 * 设置某个支付窗口的方向(目前主要解决银联窗口方向)
 *
 * 注意：
 *      只支持横屏的应用，必须在AppDelegate中实现此方法。
 **/
- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window;


/**
 * 设置SDK接入的基本信息
 * @params:appID        在爱贝注册的应用ID
 * @params:mACID        渠道号
 **/
- (BOOL)setAppId:(NSString *)appID mACID:(NSString *)mACID;


/**
 * 应用ID对应的渠道号
 * 注意：请先设置AppID，才可以重新设置标记渠道号。
 **/
- (BOOL)setAppMACID:(NSString *)mACID;


/**
 * 调起爱贝支付进行支付
 * @params:trandInfo    格式化的订单信息transdata或者transid(参照爱贝SDK服务端接入文档)
 * @params:payDelegate  支付结果处理对象
 **/
- (NSInteger)makePayForTrandInfo:(NSString *)trandInfo payDelegate:(id<IapppayKitPayRetDelegate>)payDelegate;


@end


