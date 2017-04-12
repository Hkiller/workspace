package com.drowgames.telecompay;
import android.util.Log;
import android.text.TextUtils;
import java.util.Map;
import cn.egame.terminal.paysdk.EgamePay;
import cn.egame.terminal.paysdk.EgamePayListener;
import android.app.Activity;
public class TelecomOffLinePayListener implements EgamePayListener{
    private long m_ptr;
    private static Activity m_activity;
    public TelecomOffLinePayListener(Activity activity,long ptr) {
        m_ptr = ptr;
        m_activity = activity;
    }

    public void destory() {
        m_ptr = 0;
    }
    
        @Override
    public void paySuccess(Map<String, String> params) {
        Log.i("drow", "telecompay: pay success");
        notifyResult(0/*appsvr_payment_success*/, 0, "");
    }

    @Override
    public void payFailed(Map<String, String> params, int errorInt) {
        Log.i("drow", "telecompay: pay fail");
        notifyResult(2/*appsvr_payment_failed*/, errorInt, params.get(EgamePay.PAY_PARAMS_KEY_TOOLS_ALIAS));
    }

    @Override
    public void payCancel(Map<String, String> params) {
        Log.i("drow", "telecompay: pay cancel");
        notifyResult(1/*appsvr_payment_canceled*/, 0, params.get(EgamePay.PAY_PARAMS_KEY_TOOLS_ALIAS));
    }

    private void notifyResult(final int result, final int service_result, final String error_msg) {

        if(result==2/*appsvr_payment_failed*/){

        m_activity.runOnUiThread(new Runnable() {
            public void run() {
                    try {
                        Thread.sleep(1000);
                        Log.e("drow", "telecompay: notifyResult: result=" + result + ", service_result=" + service_result + ", error_msg=" + error_msg);
                        nativeNotifyResult(m_ptr, result, service_result, error_msg);
                    } catch (InterruptedException e) {
                        e.printStackTrace(); 
                    }
                }
            });
        }
        else {
            Log.e("drow", "telecompay: notifyResult: result=" + result + ", service_result=" + service_result + ", error_msg=" + error_msg);
            nativeNotifyResult(m_ptr, result, service_result, error_msg);
        }

    }

    private native void nativeNotifyResult(long ptr, int result, int service_result, String error_msg);
}

