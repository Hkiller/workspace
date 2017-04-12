package com.drowgames.damai;
import org.json.JSONObject;
import android.content.Context;
import android.content.Intent;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import com.game.sdk.SDKManager;
import com.game.sdk.domain.LoginErrorMsg;
import com.game.sdk.domain.LogincallBack;
import com.game.sdk.domain.OnLoginListener;
import com.game.sdk.domain.OnPaymentListener;
import com.game.sdk.domain.PaymentCallbackInfo;
import com.game.sdk.domain.PaymentErrorMsg;

public class DamaiSdkManip {
    public static long _ptr;
	public static SDKManager _sdkmanager;
    
    public static void init(final Activity activity, long ptr) {
        _ptr = ptr;
        activity.runOnUiThread(new Runnable() {
                public void run() {
                    _sdkmanager = SDKManager.getInstance(activity);
		            _sdkmanager.setIsPortrait(1);
                    Log.d("drow", "SDKManager init success");
                }
            });
    }
    
	public static void fini() {
        _ptr = 0;
		_sdkmanager.recycle();//游戏退出必须调用
        _sdkmanager = null;
    }

    public static void suspend() {
		_sdkmanager.removeFloatView();
    }

    public static void resume() {
		_sdkmanager.showFloatView();
    }
    
    public static void login(Activity activity) {
        Log.d("drow", "SDKManager login begin");
        
        _sdkmanager.showLogin(activity, true, new OnLoginListener() {
				@Override
				public void loginSuccess(LogincallBack logincallback) {
					_sdkmanager.showFloatView();

                    // long logintime=logincallback.logintime;//登录回调时间戳
                    // String username=logincallback.username;//登录的用户名
                    // String token=logincallback.token;//口令

                    Log.d("drow", "SDKManager login success");
                    nativeNotifyLoginResult(_ptr, logincallback.username, logincallback.token, 0, null);                    
				}

				@Override
				public void loginError(LoginErrorMsg errorMsg) {
                    Log.d("drow", "SDKManager login fail");
                    nativeNotifyLoginResult(_ptr, null, null, errorMsg.code, errorMsg.msg);                    
				}
			});
    }

    public static void setUserInfo(String data) {
        try {
            JSONObject jsonExData = new JSONObject(data);
            _sdkmanager.submitExtendData(1, jsonExData);
        }
        catch (Exception e) {
            Log.d("drow", "setUserInfo: ");
            //处理异常
        }
    }

    public static  void doSdkPay(Activity activity, String trans, String roleid, String serverid, String money, String product_name, String product_desc) {
        Log.d("drow", "SDKManager doSdkPay begin");
        
        _sdkmanager.showPay(activity, roleid, money, serverid, product_name, product_desc, trans, new OnPaymentListener() {
                @Override
                public void paymentSuccess(PaymentCallbackInfo callbackInfo) {
                    Log.d("drow", "充值金额数：" + callbackInfo.money + " 消息提示：" + callbackInfo.msg);
                    nativeNotifyPaymentResult(_ptr, 0, null);
                }

                @Override
                public void paymentError(PaymentErrorMsg errorMsg) {
                    Log.d("drow", "充值失败：code:" + errorMsg.code + "  ErrorMsg:" + errorMsg.msg + "  预充值的金额：" + errorMsg.money);
                    nativeNotifyPaymentResult(_ptr, errorMsg.code, errorMsg.msg);
                }
            });
	}

    private native static void nativeNotifyLoginResult(long ptr, String username, String access_token, int error, String error_msg);
    private native static void nativeNotifyPaymentResult(long ptr, int error, String error_msg);
}


