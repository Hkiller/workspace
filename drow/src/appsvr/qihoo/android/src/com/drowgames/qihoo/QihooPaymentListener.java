package com.drowgames.qihoo;
import org.json.JSONObject;
import org.json.JSONException;
import android.util.Log;
import android.text.TextUtils;
import com.qihoo.gamecenter.sdk.common.IDispatcherCallback;

public class QihooPaymentListener implements IDispatcherCallback {
    private long m_ptr;
    
    public QihooPaymentListener(long ptr) {
        m_ptr = ptr;
    }

    public void destory() {
        m_ptr = 0;
    }

    public void onFinished(String paramString) {
		// error_code 状态码： 0 支付成功， -1 支付取消， 1 支付失败， -2 支付进行中, 4010201和4009911 登录状态已失效，引导用户重新登录
        // error_msg 状态描述

        try {
            JSONObject joRes = new JSONObject(paramString);
            int errorCode = joRes.getInt("error_code");
            nativeNotifyResult(m_ptr, errorCode, joRes.getString("error_msg"));
        }
        catch(JSONException e) {
            e.printStackTrace();
            nativeNotifyResult(m_ptr, -1, "unknown error");
        }
    }

    private native void nativeNotifyResult(long ptr, int errorCode, String error_msg);
}

