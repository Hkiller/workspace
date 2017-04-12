package com.drowgames.facebook;
import org.json.JSONObject;
import org.json.JSONException;
import android.util.Log;
import android.text.TextUtils;

import com.facebook.CallbackManager; 
import com.facebook.FacebookCallback; 
import com.facebook.AccessToken;
import com.facebook.login.LoginResult;
import com.facebook.FacebookException;

public class FacebookLoginListener {
    private long m_ptr;
    
    public FacebookLoginListener(long ptr) {
        m_ptr = ptr;
    }

    // @Override
    // public void onSuccess(LoginResult loginResult) {
    //      // App code
    //     Log.e("drow", "facebook: onSuccess: login success enter 1");

    //     AccessToken accesstoken = AccessToken.getCurrentAccessToken();
    //     Log.e("drow", "facebook: onSuccess: login success accesstoken=" + accesstoken.toString());
    //     Log.e("drow", "facebook: onSuccess: login success enter 2");
    //     nativeNotifyResult(m_ptr,0, accesstoken.toString(), 0);
    // }

    // @Override
    // public void onCancel() {
    //      // App code
    //     Log.e("drow", "facebook: onCancel: login cancel");

    //     nativeNotifyResult(m_ptr, 0, "", 1);
    // }

    // @Override
    // public void onError(FacebookException exception) {
    //      // App code   
    //     Log.e("drow", "facebook: onError: login fail");

    //     nativeNotifyResult(m_ptr, 0, "", 2);
    //  }

    public void destory() {
        m_ptr = 0;
    }
    public void nativeNotifyLoginResult(int expires_in, String access_token, int error)
    {
        nativeNotifyResult(m_ptr,expires_in,access_token,error);
    }
    private native void nativeNotifyResult(long ptr, int expires_in, String access_token, int error);
}

