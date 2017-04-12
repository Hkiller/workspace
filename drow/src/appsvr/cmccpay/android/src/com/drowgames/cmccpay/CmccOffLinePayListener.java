package com.drowgames.cmccpay;
import android.util.Log;
import android.text.TextUtils;
import cn.cmgame.billing.api.BillingResult;
import cn.cmgame.billing.api.GameInterface.IPayCallback;
public class CmccOffLinePayListener implements IPayCallback{
    private long m_ptr;

    public CmccOffLinePayListener(long ptr) {
        m_ptr = ptr;
    }

    public void destory() {
        m_ptr = 0;
    }
    
      @Override
    public void onResult(int resultCode, String billingIndex, Object obj) {
        switch (resultCode) {
        case BillingResult.SUCCESS://success
            Log.i("drow", "Unicom: pay success");
            notifyResult(0/*appsvr_payment_success*/, 0, "");
            break;
        case BillingResult.FAILED://fail
            Log.i("drow", "Unicom: pay fail");
            notifyResult(2/*appsvr_payment_failed*/, 0, "");
            break;
        case BillingResult.CANCELLED://cancel
            Log.i("drow", "Unicom: pay cancel");
            notifyResult(1/*appsvr_payment_canceled*/, resultCode, billingIndex);
            break;
        default:
            break;
        }

    }
	

    private void notifyResult(final int result, final int service_result, final String error_msg) {
        Log.e("drow", "iapppay: notifyResult: result=" + result + ", service_result=" + service_result + ", error_msg=" + error_msg);
        nativeNotifyResult(m_ptr, result, service_result, error_msg);
    }

    private native void nativeNotifyResult(long ptr, int result, int service_result, String error_msg);
}

