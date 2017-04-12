package com.drowgames.iapppay;
import android.util.Log;
import android.text.TextUtils;
import com.iapppay.sdk.main.IAppPay;
import com.iapppay.sdk.main.IAppPayOrderUtils;
import com.iapppay.interfaces.callback.IPayResultCallback;

public class IAppPayListener implements IPayResultCallback {
    private long m_ptr;

    public IAppPayListener(long ptr) {
        m_ptr = ptr;
    }

    public void destory() {
        m_ptr = 0;
    }
    
    public void onPayResult(int resultCode, String signvalue, String resultInfo) {
        if (m_ptr == 0) return;
        
        switch (resultCode) {
        case IAppPay.PAY_SUCCESS:
            boolean isPaySuccess = IAppPayOrderUtils.checkPayResult(signvalue , IAppPayUtils.s_platp_key);
            if(isPaySuccess) {
                /*支付成功，验签成功 */
                Log.i("drow", "iapppay: pay success");
                
                notifyResult(0/*appsvr_payment_success*/, 0, "");
            }
            else {
                /*支付成功，验签失败 */
                Log.e("drow", "iapppay: pay success, but verify signature error");

                notifyResult(2/*appsvr_payment_failed*/, 0, "");
            }
            break;
        case IAppPay.PAY_ERROR:
            /*支付失败 */
            Log.e("drow", "iapppay: pay fail, RetCode=" + resultCode + ", ErrorMsg=" + resultInfo);
            notifyResult(2/*appsvr_payment_failed*/, resultCode, resultInfo);
            break;
        default:
            /*支付取消 */
            Log.e("drow", "iapppay: pay cancel, RetCode=" + resultCode + ", ErrorMsg=" + resultInfo);
            notifyResult(1/*appsvr_payment_canceled*/, resultCode, resultInfo);
            break;
        }
    }

    private void notifyResult(final int result, final int service_result, final String error_msg) {
        Log.e("drow", "iapppay: notifyResult: result=" + result + ", service_result=" + service_result + ", error_msg=" + error_msg);
        nativeNotifyResult(m_ptr, result, service_result, error_msg);
    }

    private native void nativeNotifyResult(long ptr, int result, int service_result, String error_msg);
}

