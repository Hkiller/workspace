package com.drowgames.googlepay;
import android.util.Log;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import org.json.JSONObject;

public class GooglepayOffLineListener{
    private long m_ptr;

    public GooglepayOffLineListener(long ptr) {
        m_ptr = ptr;
    }

    public void destory() {
        m_ptr = 0;
    }
    
    public void notifyResult(final int result, final int service_result, final String error_msg) {
        Log.e("drow", "googlepay: notifyResult: result=" + result + ", service_result=" + service_result + ", error_msg=" + error_msg);
        nativeNotifyPayResult(m_ptr, result, service_result, error_msg);
    }

    public void notifyProductInfoResult(final String product_id, final String price) {
        Log.e("drow", "googlepay: notifyProductInfoResult: product_id=" + product_id + ", price=" + price);
        nativeNotifyProductInfoResult(m_ptr, product_id, price);
    }

    private native void nativeNotifyPayResult(long ptr, int result, int service_result, String error_msg);
    private native void nativeNotifyProductInfoResult(long ptr, String product_id, String price);

}

