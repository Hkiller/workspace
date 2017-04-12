//
//  NDJWorkSpace.h
//  NDJFramework
//
//  Created by 陈祖发 on 16/5/31.
//  Copyright © 2016年 陈祖发. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@protocol NDJWorkSpaceDelegate <NSObject>

@optional
- (void)ndjAdsWillShow;
- (void)ndjAdsDidShow;
- (void)ndjAdsWillHide;
- (void)ndjAdsDidHide;
/**
 *  开始播放
 */
- (void)ndjAdsVideoStarted:(NSString *)adType;

/**
 *  播放结束
 */
- (void)ndjAdsVideoEnded:(NSString *)adType;

/**
 *  跳过视频播放
 */
- (void)ndjAdsVideoSkipped:(NSString *)adType;

/**
 *  播放错误
 */
- (void)ndjAdsVideoPlayError:(NSString *)adType;

/**
 *  点击下载，跳出应用触发
 */
- (void)ndjAdsWillLeaveApp;

/**
 *  当前是否有可播放视频
 *
 *  @param isCanshow YES 有、NO 无。
 *  @param error     错误信息
 */
- (void)ndjCanshowVideo:(BOOL)isCanshow error:(NSDictionary*)error;

@end

@interface NDJWorkSpace : NSObject

@property (nonatomic, weak) id<NDJWorkSpaceDelegate> delegate;

+ (instancetype) defaultWorkSpace;

/**
 *  判断系统版本是否支持 ，7.0开始
 *
 *  @return return value description
 */
+ (BOOL)isSupported;

/**
 *  获取sdk版本号
 *
 *  @return <#return value description#>
 */
+ (NSString *)getSdkVersion;

/**
 *  初始化
 *
 *  @param appId          平台应用id
 *  @param viewController 控制器
 *  @param successedBlcok 成功回调
 *  @param errorBlcok     错误回调
 */
- (void)startWithAppId:(NSString *)appId andViewController:(UIViewController*)viewController successed:(void (^)(NSDictionary * resultDic))successedBlcok error:(void (^)(NSDictionary * errorDic))errorBlcok;

/**
 *  初始化
 *
 *  @param appId          平台应用id
 *  @param successedBlcok 成功回调
 *  @param errorBlcok     错误回调
 */
- (void)startWithAppId:(NSString *)appId successed:(void (^)(NSDictionary * resultDic))successedBlcok error:(void (^)(NSDictionary * errorDic))errorBlcok;

/**
 *
 *  @param viewController 当前视图控制器
 */
- (void)setViewController:(UIViewController *)viewController;

/**
 *  只播放已缓存好的视频
 *  默认是都可以播放，不管是缓存好的，还是没缓存好的
 *  设置这个值的时候，请在调用canshow和show之前。
 *  @param value YES 只播放已缓存好的视频  / NO 都可以播放
 */
- (void)setPlayCachedVideo:(BOOL)value;

/**
 *  设置视频只在WiFi环境下缓存  还是WiFi和移动网络都能缓存
 *  默认只在WiFi下缓存
 *
 *  @param value YES WiFi环境下缓存/ NO WiFi和移动网络都缓存
 */
- (void)setDownloadOnlyInWifi:(BOOL)value;


/**
 *  设置广告位
 *
 *  @param placementId <#placementId description#>
 */
- (void)setPlaceMentId:(NSString *)placementId;

/**
 *  查询是否有视频可播放 ，会触发回调
 *  调用前先设置placement
 */
- (void)queryVideoAD;

/**
 *  判断是否有可展示视频
 *
 *  @param placementId 广告位id
 *  @param errorBlcok  错误回调
 *
 *  @return YES 有/ NO没有
 */
- (BOOL)canShowWithPlacementId:(NSString *)placementId error:(void (^)(NSDictionary * errorDic))errorBlcok;

/**
 *  播放视频
 *  @param errorBlcok  错误回调
 *
 *  @return YES 播放成功，NO 播放失败
 */
- (void)showWitherror:(void (^)(NSDictionary * errorDic))errorBlcok;

/**
 *  设置渠道号
 *
 *  @param channel <#channel description#>
 */
- (void)setChannelid:(NSString *)channel;

/**
 *  清空视频缓存
 */
- (void)clearVideoCache;

@end
