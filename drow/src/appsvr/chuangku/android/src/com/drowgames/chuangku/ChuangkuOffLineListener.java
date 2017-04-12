package com.drowgames.chuangku;
import android.util.Log;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import org.json.JSONObject;
import com.ck.sdk.CKCode;
import com.ck.sdk.ICKSDKListener;
import com.ck.sdk.InitResult;
import com.ck.sdk.LoginResult;
import com.ck.sdk.PayParams;
import com.ck.sdk.PayResult;
import com.ck.sdk.plugin.CKPay;
import com.ck.sdk.CKSDK;

public class ChuangkuOffLineListener implements ICKSDKListener{
    private long m_ptr;
    private static Activity m_activity;
    private boolean mInit = false;
    private static final String TAG = "ChuangkuOffLineListener";

    public ChuangkuOffLineListener(Activity activity,long ptr) {
        m_ptr = ptr;
        m_activity = activity;
        mInit = true;
    }

    public void destory() {
        m_ptr = 0;
    }

    /**
     * 初始化成功
     */
    @Override
    public void onInitResult(InitResult result) {
        // 下面处理初始化成功后的逻辑
        //只有初始化成功了游戏才能调用支付方法
        mInit = true;
        Log.d(TAG, "Init finish & Succ ");
    }

        /**
     * 支付方法
     * 
     * @param context
     * @param params
     */
    public void StartIap(Activity activity, PayParams params) {

        if (mInit) {
            CKPay.getInstance().pay(params);// 调用创酷sdk 支付
        } else {
            m_activity.runOnUiThread(new Runnable() {

                @Override
                public void run() {

                }
            });
        }
    }

    /**
     * 支付成功
     */
    @Override
    public void onPayResult(final PayResult payResult) {
        Log.d(TAG, "SDK pay succ, msg : " + payResult);

        // 下面处理支付成功后的逻辑，发放道具代码
        m_activity.runOnUiThread(new Runnable() {

            @Override
            public void run() {
                Log.i("drow", "chuangku: pay success");
                notifyResult(0/*appsvr_payment_success*/, 0, "");
            }
        });

    }


    /**
     * 失败回调
     */
    @Override
    public void onResult(int code, String msg) {
        switch (code) {
        // 初始化失败
        case CKCode.CODE_INIT_FAIL: {
            mInit = false;
            nativeNotifyChuangkuInitResult(m_ptr,0);

            m_activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                }
            });
            Log.e(TAG, "SDK Init failed, msg : " + msg);
            break;
        }
        case CKCode.CODE_LOGIN_FAIL: {
            //登陆失败，网游才需要处理
            Log.e(TAG, "login falied code  " + code + "msg " + msg);
            break;
        }
        // 支付失败
        case CKCode.CODE_PAY_FAIL: {
            // 下面出事支付失败逻辑，没有可以不写
            Log.i("drow", "chuangku: pay fail");
            notifyResult(2/*appsvr_payment_failed*/, code, msg);
            break;
        }
        default: {
            Log.e(TAG, "code  " + code + "msg " + msg);
            break;
        }
        }
    }

    @Override
    public void onLoginResult(LoginResult arg0) {
        //登陆成功，网游才需要处理
    }

    @Override
    public void onLogout() {
        //退出登陆，网游才需要处理
    }

    @Override
    public void onSwitchAccount() {

    }

    @Override
    public void onSwitchAccount(LoginResult arg0) {

    }

    @Override
    public void onAuthResult(boolean arg0,int arg1,String arg2,String arg3){

    }

    public void notifySupportExitResult(final int result) {
         Log.e(TAG, "notifySupportExitResult  enter");
            nativeNotifyExitGameSupportResult(m_ptr, result);
    }
    
    private void notifyResult(final int result, final int service_result, final String error_msg) {
        Log.e("drow", "chuangku: notifyResult: result=" + result + ", service_result=" + service_result + ", error_msg=" + error_msg);
        nativeNotifyPayResult(m_ptr, result, service_result, error_msg);
    }
    
    private native void nativeNotifyExitCancelResult(long ptr);
    private native void nativeNotifyPayResult(long ptr, int result, int service_result, String error_msg);
    private native void nativeNotifyExitGameSupportResult(long ptr, int result);
    private native void nativeNotifyChuangkuInitResult(long ptr, int is_succ);

}

