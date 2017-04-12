package com.drowgames.qihoo;
import android.content.Context;
import android.content.Intent;
import android.app.Activity;
import com.qihoo.gamecenter.sdk.common.IDispatcherCallback;
import com.qihoo.gamecenter.sdk.matrix.Matrix;
import com.qihoo.gamecenter.sdk.activity.ContainerActivity;
import com.qihoo.gamecenter.sdk.protocols.ProtocolConfigs;
import com.qihoo.gamecenter.sdk.protocols.ProtocolKeys;
import android.os.Bundle;
import java.util.Scanner;

public class QihooSdkManip {
    public static void init(final Activity activity) {
        activity.runOnUiThread(new Runnable() {
                public void run() {
                    Matrix.init(activity);
                }
            });
    }
    
	public static void onDestroy(final Activity activity) {
		Matrix.destroy(activity);
    }

    /**
     * 使用 360SDK 的登录接口, 生成 intent 参数 *
     * @param isLandScape 是否横屏显示登录界面
     */
    public static void login(long ctx,
                             Activity activity,
                             boolean isLandScape,
                             boolean isShowClose,
                             boolean isShowSwitchButton,
                             boolean isHideWellcome,
                             String backgroundPicPath,
                             boolean backgroundPicInAssets,
                             boolean isNeedActivationCode,
                             boolean isAutoLoginHideUI,
                             boolean isShowDlgOnFailedAutoLogin,
							 boolean isRelogin)
    {
        Intent intent = new Intent(activity, ContainerActivity.class); // 必需参数,使用360SDK的登录模块
        
        // 可选参数,360SDK 界面是否以横屏显示,默认为 true,横屏
        intent.putExtra(ProtocolKeys.IS_SCREEN_ORIENTATION_LANDSCAPE, isLandScape);
        //可选参数,是否显示关闭按钮,默认不显示
        intent.putExtra(ProtocolKeys.IS_LOGIN_SHOW_CLOSE_ICON, isShowClose);

        // 可选参数,是否在自动登录的过程中显示切换账号按钮,默认为false
        intent.putExtra(ProtocolKeys.IS_SHOW_AUTOLOGIN_SWITCH, isShowSwitchButton);
        // 可选参数,是否隐藏欢迎界面
        intent.putExtra(ProtocolKeys.IS_HIDE_WELLCOME, isHideWellcome);
        /*
          指定界面背景(可选参数):
          1.ProtocolKeys.UI_BACKGROUND_PICTRUE 使用的系统路径,如/sdcard/1.png
          2.ProtocolKeys.UI_BACKGROUND_PICTURE_IN_ASSERTS 使用的assest中的图片资源, 
          如传入 bg.png 字符串,就会在 assets 目录下加载这个指定的文件
          3.图片大小不要超过5M,尺寸不要超过1280x720,后缀只能是jpg、jpeg或png
        */
        if (backgroundPicInAssets) {
            // 可选参数,登录界面的背景图片路径,必须是本地图片路径
            intent.putExtra(ProtocolKeys.UI_BACKGROUND_PICTURE_IN_ASSERTS, backgroundPicPath);
        }
        else {
            // 可选参数,指定assets中的图片路径,作为背景图
            intent.putExtra(ProtocolKeys.UI_BACKGROUND_PICTRUE, backgroundPicPath);
        }
        
        // 可选参数,是否需要用户输入激活码,用于游戏内测阶段。
        // 如果不需激活码相关逻辑,客户传 false 或者不传入该参数。
        intent.putExtra(ProtocolKeys.NEED_ACTIVATION_CODE, isNeedActivationCode);

        //-- 以下参数仅仅针对自动登录过程的控制
        // 可选参数,自动登录过程中是否不展示任何UI,默认展示。
        intent.putExtra(ProtocolKeys.IS_AUTOLOGIN_NOUI, isAutoLoginHideUI);
        // 可选参数,静默自动登录失败后是否显示登录窗口,默认不显示
        intent.putExtra(ProtocolKeys.IS_SHOW_LOGINDLG_ONFAILED_AUTOLOGIN, isShowDlgOnFailedAutoLogin);
        
		IDispatcherCallback callback = new QihooLoginListener(ctx);
		if(isRelogin)
		{
			intent.putExtra(ProtocolKeys.FUNCTION_CODE, ProtocolConfigs.FUNC_CODE_SWITCH_ACCOUNT);
			Matrix.invokeActivity(activity, intent, callback);
		}else
		{
			intent.putExtra(ProtocolKeys.FUNCTION_CODE, ProtocolConfigs.FUNC_CODE_LOGIN);
			Matrix.execute(activity, intent, callback);
		}
    }

	//2 、 参数名，以 ProtocolKeys 中定义的常量为准。
	/**
	* 使用 360SDK 的支付接口
	*
	* @param isLandScape 是否横屏显示支付界面
	* @param isFixed 是否定额支付
	*/
	 public static  void doSdkPay(long ctx,
								  Activity activity,
								  boolean isLandScape,
								  //boolean isFixed,
								  String backgroundPicPath,
								  String accessToken,
								  String qihooUserId,
								  int moneyAmount,
								  String productName,
								  int productId,
								  String notifyUri,
								  String appName,
								  String appUserId,
								  String appUserName,
								  String appOrderId
								  ) 
    {
		// 支付基础参数
		Bundle bundle = new Bundle();
		
		// 界面相关参数，360SDK 界面是否以横屏显示。
		bundle.putBoolean(ProtocolKeys.IS_SCREEN_ORIENTATION_LANDSCAPE, isLandScape);
		// 可选参数，登录界面的背景图片路径，必须是本地图片路径
		bundle.putString(ProtocolKeys.UI_BACKGROUND_PICTRUE, backgroundPicPath);
		// *** 以下非界面相关参数 ***
		// 设置 QihooPay 中的参数。
		// 必需参数，用户 access token，要使用注意过期和刷新问题，最大 64 字符。
		bundle.putString(ProtocolKeys.ACCESS_TOKEN,accessToken);
		// 必需参数，360 账号 id。
		bundle.putString(ProtocolKeys.QIHOO_USER_ID, qihooUserId);
		// 必需参数，所购买商品金额, 以分为单位。金额大于等于 100 分，360SDK 运行定额支付流程； 金额数为 0，360SDK 运行不定额支付流程。
		bundle.putString(ProtocolKeys.AMOUNT, String.valueOf(moneyAmount) );
		// 必需参数，所购买商品名称，应用指定，建议中文，最大 10 个中文字。
		bundle.putString(ProtocolKeys.PRODUCT_NAME, productName);
		// 必需参数，购买商品的商品 id，应用指定，最大 16 字符。
		bundle.putString(ProtocolKeys.PRODUCT_ID, String.valueOf(productId));
		// 必需参数，应用方提供的支付结果通知 uri，最大 255 字符。360 服务器将把支付接口回调给该 uri，具体协议请查看文档中，支付结果通知接口–应用服务器提供接口。
		bundle.putString(ProtocolKeys.NOTIFY_URI, notifyUri);
		// 必需参数，游戏或应用名称，最大 16 中文字。
		bundle.putString(ProtocolKeys.APP_NAME, appName);
		// 必需参数，应用内的用户名，如游戏角色名。 若应用内绑定 360 账号和应用账号，则可用 360 用户名，最大 16 中文字。（充值不分区服，充到统一的用户账户，各区服角色均可使用）。
		bundle.putString(ProtocolKeys.APP_USER_NAME, appUserName);
		// 必需参数，应用内的用户 id。
		
		// 若应用内绑定 360 账号和应用账号，充值不分区服，充到统一的用户账户，各区服角色均可使用，则可用 360 用户 ID 最大 32 字符。
		bundle.putString(ProtocolKeys.APP_USER_ID, appUserId );
		// 必需参数，应用订单号，应用内必须唯一，最大 32 字符。
		bundle.putString(ProtocolKeys.APP_ORDER_ID, appOrderId  );

		Intent intent = new Intent(activity, ContainerActivity.class);
		intent.putExtras(bundle);

		// 必需参数，使用 360SDK 的支付模块。
		intent.putExtra(ProtocolKeys.FUNCTION_CODE, ProtocolConfigs.FUNC_CODE_PAY);
		// 可选参数，登录界面的背景图片路径，必须是本地图片路径
		intent.putExtra(ProtocolKeys.UI_BACKGROUND_PICTRUE, "");

		IDispatcherCallback callback = new QihooPaymentListener(ctx);
		Matrix.invokeActivity(activity, intent, callback);
	}
}


