package com.drowgames.unicompay;
import android.util.Log;
import android.text.TextUtils;
import com.unicom.dcLoader.Utils.UnipayPayResultListener;
public class UnicomOffLinePayListener implements UnipayPayResultListener{
    private long m_ptr;

    public UnicomOffLinePayListener(long ptr) {
        m_ptr = ptr;
    }

    public void destory() {
        m_ptr = 0;
    }
    
	//@Override
	public void PayResult(String arg0, int arg1, int arg2, String arg3) {
		switch (arg1) {
		case 1://success
			Log.i("drow", "Unicom: pay success");
			notifyResult(0/*appsvr_payment_success*/, 0, "");
			break;
		case 2://fail
			Log.i("drow", "Unicom: pay fail");
			notifyResult(2/*appsvr_payment_failed*/, 0, "");
			break;
		case 3://cancel
			Log.i("drow", "Unicom: pay cancel");
			notifyResult(1/*appsvr_payment_canceled*/, arg1, arg0);
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

