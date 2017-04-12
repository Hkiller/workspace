package com.drowgames.facebook;
import android.content.Context;
import android.content.Intent;
import android.app.Activity;

import java.util.Arrays;

import com.drowgames.helper.DrowActivity;
import android.os.Bundle;
import android.content.Intent;
import com.facebook.FacebookSdk; 
import com.facebook.AccessToken;
import com.facebook.login.LoginResult;
import com.facebook.CallbackManager; 
import com.facebook.login.LoginManager;
import com.facebook.FacebookCallback; 
import android.util.Log;
import com.facebook.FacebookException;
import android.app.Application;


public class FacebookManip {
    static Activity m_activity;
    static long _ptr;
    static CallbackManager callbackManager;
    static FacebookLoginListener listener;
    public static void init(final Activity activity, final long ptr) {
        activity.runOnUiThread(new Runnable() {
                public void run() {
                	m_activity = activity;
                    _ptr = ptr;
                    listener = new FacebookLoginListener(ptr);
                    FacebookSdk.sdkInitialize(activity.getApplicationContext());
                 	callbackManager = CallbackManager.Factory.create();
                 	LoginManager.getInstance().registerCallback(callbackManager,
                       new FacebookCallback<LoginResult>(){
                            @Override
                            public void onSuccess(LoginResult loginResult) {
                                 // App code
                                Log.e("drow", "facebook: onSuccess: login success enter 1");

                                AccessToken accesstoken = AccessToken.getCurrentAccessToken();
                                Log.e("drow", "facebook: onSuccess: login success accesstoken=" + accesstoken.toString());
                                Log.e("drow", "facebook: onSuccess: login success enter 2");
                                listener.nativeNotifyLoginResult(0,  accesstoken.getToken(), 0);
                            }

                            @Override
                            public void onCancel() {
                                 // App code
                                Log.e("drow", "facebook: onCancel: login cancel");

                                listener.nativeNotifyLoginResult(0, "", 1);
                            }

                            @Override
                            public void onError(FacebookException exception) {
                                 // App code   
                                Log.e("drow", "facebook: onError: login fail");

                                listener.nativeNotifyLoginResult(0, "", 2);
                             }
                       });
                }
            });
    }
   
    public static void fini() {
        _ptr = 0;
    } 
    public static void login(boolean isRelogin)
    {
     	LoginManager.getInstance().
        logInWithReadPermissions(m_activity, Arrays.asList("public_profile", "user_friends"));
    }

    public static void onActivityResult(Object intent,int requestCode, int resultCode){
        System.out.println("facebook sdk onActivityResult enter");
        if (callbackManager != null) {
            callbackManager.onActivityResult(requestCode, resultCode, (Intent)intent);
        }
    }
}


