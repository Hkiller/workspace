//
//  TGSDK.h
//  TGSDK
//
//  Created by SunHan on 9/7/15.
//  Copyright (c) 2015 SoulGame. All rights reserved.
//

#import <Foundation/Foundation.h>

#define kTGSDKServiceResultErrorInfo @"kTGSDKServiceResultErrorInfo"
typedef void (^TGSDKServiceResultCallBack)(BOOL success, id _Nullable tag, NSDictionary* _Nullable result);

typedef enum {
    TGAdPlatformTG,
}TGAdPlatform;

typedef enum {
    TGAdTypeNone,
    TGAdTypeCP,
    TGAdType3rdCP,
    TGAdType3rdPop,
    TGAdType3rdVideo,
    TGADType3rdAward,
    TGAdType3rdNative
}TGAdType;


@protocol TGPreloadADDelegate <NSObject>
@optional
- (void) onPreloadSuccess:(NSString* _Nullable)result;

- (void) onPreloadFailed:(NSString* _Nullable)result WithError:(NSError* _Nullable) error;

- (void) onCPADLoaded:(NSString* _Nonnull) result;

- (void) onVideoADLoaded:(NSString* _Nonnull) result;

@end


@protocol TGADDelegate <NSObject>
@optional
- (void) onShowSuccess:(NSString* _Nonnull)result;

- (void) onShowFailed:(NSString* _Nonnull)result WithError:(NSError* _Nullable)error;

- (void) onADComplete:(NSString* _Nonnull)result;

- (void) onADClick:(NSString* _Nonnull)result;

- (void) onADClose:(NSString* _Nonnull)result;

@end


@protocol TGRewardVideoADDelegate <TGADDelegate>
@optional
- (void) onADAwardSuccess:(NSString* _Nonnull)result;

- (void) onADAwardFailed:(NSString* _Nonnull)result WithError:(NSError* _Nullable)error;

@end


@interface TGSDK : NSObject

@property (strong, nonatomic) NSString* _Nonnull appID;
@property (strong, nonatomic) NSString* _Nonnull publisherID;
@property (strong, nonatomic) NSString* _Nonnull channelID;
@property (strong, nonatomic, readonly) NSString* _Nonnull udid;
@property (strong, nonatomic, readonly) NSString* _Nullable tgid;
@property (strong, nonatomic, readonly) NSString* _Nullable userRegisterDate;
@property (nonatomic, readonly) BOOL debugEnv;
@property (nonatomic) BOOL enableLog;

+(TGSDK* _Nonnull)sharedInstance;

//初始化函数
+ (void) setDebugModel:(BOOL)debug;

+ (void) enableTestServer;

+ (void) initialize:(NSString* _Nonnull)appid
          channelID:(NSString* _Nonnull)channelid
           callback:(TGSDKServiceResultCallBack _Nullable)cb;

+ (void) initialize:(NSString* _Nonnull)appid
           callback:(TGSDKServiceResultCallBack _Nullable)cb;

+ (void) initialize:(TGSDKServiceResultCallBack _Nullable)cb;

+(void)setSDKConfig:(NSString* _Nullable)val forKey:(NSString* _Nonnull)key;
+(NSString* _Nullable)getSDKConfig:(NSString* _Nonnull)key;

//获取sceneParams
+(NSArray *_Nullable)getSceneParams:(NSString *_Nonnull)sceneID forKey:(NSString* _Nonnull)key;

//平台注册
+(void)userPlatformRegister:(NSString* _Nonnull)userName
                   password:(NSString* _Nonnull)userPassword
                        tag:(id _Nullable)tag
                   callBack:(TGSDKServiceResultCallBack _Nullable)cb;

//第三方注册
+(void)userPartnerRegister:(NSString* _Nonnull)puid
                   partner:(NSString* _Nonnull)partner
                       tag:(id _Nullable)tag
                  callBack:(TGSDKServiceResultCallBack _Nullable)cb __attribute__((deprecated));

//默认注册 － 暂时不能使用
//+(void)userDefaultRegister:(id)tag callBack:(TGSDKServiceResultCallBack)cb;

//平台登陆
+(void)userPlatformLogin:(NSString* _Nonnull)userName
                password:(NSString* _Nonnull)userPassword
                     tag:(id _Nullable)tag
                   callBack:(TGSDKServiceResultCallBack _Nullable)cb;

//第三方登陆
+(void)userPartnerLogin:(NSString* _Nonnull)puid
                partner:(NSString* _Nonnull)partner
                    tag:(id _Nullable)tag
               callBack:(TGSDKServiceResultCallBack _Nullable)cb __attribute__((deprecated));

//第三方绑定
//就是登录和注册的合体版本
+(void)userPartnerBind:(NSString* _Nonnull)puid
               partner:(NSString* _Nonnull)partner
                   tag:(id _Nullable)tag
              callBack:(TGSDKServiceResultCallBack _Nullable)cb;

/**************************   广告相关  ******************************/

/*游戏在启动、登陆完成后，调用预加载接口进行广告的预加载*/
+(int) isWIFI;
+(void) preloadAd:(id<TGPreloadADDelegate> _Nullable) delegate;
+(void) preloadAdOnlyWIFI:(id<TGPreloadADDelegate> _Nullable)delegate;

+(BOOL)couldShowAd:(NSString* _Nonnull)scene;

/*当开始给用户显示广告的时候调用，返回值如果是NSString，则是预加载没有完成或者没有调用预加载，如果返回值是NSData，则是图片的数据。同时发送counter cp_adview*/
+(void)setADDelegate:(id<TGADDelegate> _Nullable)delegate;
+(void)setRewardVideoADDelegate:(id<TGRewardVideoADDelegate> _Nullable)delegate;
+(void)showAd: (NSString* _Nonnull)scene;
+(void)reportAdRejected:(NSString* _Nonnull)sceneId;
+(void)showAdScene:(NSString* _Nonnull)scene;

+(NSString* _Nullable)getCPImagePath:(NSString* _Nonnull)scene;
+(void)showCPView:(NSString* _Nonnull)scene;
+(void)reportCPClick:(NSString* _Nonnull)scene;
+(void)reportCPClose:(NSString* _Nonnull)scene;

/**************************   数据追踪  ******************************/
+ (void)sendCounter:(NSString* _Nonnull)name metaData:(NSDictionary* _Nullable)md;
+ (void)sendCounter:(NSString* _Nonnull)name metaDataJson:(NSString* _Nullable)mdJson;
+ (void)paymentCounter:(nonnull NSString*)productId
           WithMethod:(nullable NSString*)method
      AndTransactionId:(nullable NSString*)trans
           AndCurrency:(nullable NSString*)currency
              AndPrice:(float)price
           AndQuantity:(int)quantity
             AndAmount:(float)amount
        AndGoodsAmount:(int)goodsAmount;

/**************************   错误日志上传  ******************************/

@end
