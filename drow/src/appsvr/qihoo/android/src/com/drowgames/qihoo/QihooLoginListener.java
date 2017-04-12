package com.drowgames.qihoo;
import org.json.JSONObject;
import org.json.JSONException;
import android.util.Log;
import android.text.TextUtils;
import com.qihoo.gamecenter.sdk.common.IDispatcherCallback;

public class QihooLoginListener implements IDispatcherCallback {
    private long m_ptr;
    
    public QihooLoginListener(long ptr) {
        m_ptr = ptr;
    }

    public void destory() {
        m_ptr = 0;
    }

    public void onFinished(String paramString) {
        //"data": {
        //    "expires_in": "36000",
        //        "scope": "",
        //        "refresh_token": "",
        //        "access_token": "6461171100c3bfa3cba24cc332d7d78b311d2bf590f4877c9" // token
        //        },
        //    "errno": 0
        //}

        try {
            JSONObject joRes = new JSONObject(paramString);
            int errno = joRes.getInt("errno");
            if (errno == 0) {
                JSONObject joData = joRes.getJSONObject("data");
                nativeNotifyResult(m_ptr, joData.getInt("expires_in"), joData.getString("access_token"), errno);
            }
            else {
                nativeNotifyResult(m_ptr, 0, "", errno);
            }
        }
        catch(JSONException e) {
            Log.e("drow", "qiho: onLoginFinished: parse result " + paramString + " fail", e);
            nativeNotifyResult(m_ptr, 0, "", -1);
        }
    }

    private native void nativeNotifyResult(long ptr, int expires_in, String access_token, int error);
}

