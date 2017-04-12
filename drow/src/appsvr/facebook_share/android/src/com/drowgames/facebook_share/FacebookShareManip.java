package com.drowgames.facebook_share;

import android.content.Intent;
import android.net.Uri;
import android.app.Activity;
import android.util.Log;
import com.facebook.FacebookSdk; 
import com.facebook.CallbackManager; 
import com.facebook.FacebookCallback; 
import com.facebook.FacebookException;
import com.facebook.share.Sharer;
import com.facebook.share.model.ShareLinkContent;
import com.facebook.share.widget.ShareDialog;;

public class FacebookShareManip {
    static Activity m_activity;
    static long _ptr;
    static CallbackManager callbackManager;
    static FacebookShareListener listener;
    static ShareDialog shareDialog;
    static String LOG_TAG = "FacebookShareManip";
    
    public static void init(final Activity activity, final long ptr) {
        activity.runOnUiThread(new Runnable() {
                public void run() {
                	m_activity = activity;
                    _ptr = ptr;
                    FacebookSdk.sdkInitialize(m_activity.getApplicationContext());
                    callbackManager = CallbackManager.Factory.create();
                    shareDialog = new ShareDialog(m_activity);
                    // this part is optional

                    shareDialog.registerCallback(callbackManager, new FacebookCallback<Sharer.Result>() {

                        @Override
                        public void onSuccess(Sharer.Result result) {
                            //Log.d(LOG_TAG, "success");
                        }

                        @Override
                        public void onError(FacebookException error) {
                            //Log.d(LOG_TAG, "error");
                        }

                        @Override
                        public void onCancel() {
                            //Log.d(LOG_TAG, "cancel");
                        }
                    });
                }
            });
    }
   
    public static void fini() {
        _ptr = 0;
    } 
    public static void onShare(final String contentURL,final String contentTitle,final String imageURL){
        Log.e("drow", "facebook onShare: onShare enter");
        if (ShareDialog.canShow(ShareLinkContent.class)) {
            ShareLinkContent linkContent = new ShareLinkContent.Builder()
                    .setContentTitle(contentTitle)
                    .setContentUrl(Uri.parse(contentURL))
                    .setImageUrl(Uri.parse(imageURL))
                    .build();

            shareDialog.show(linkContent);
        }
    }

    public static void onActivityResult(Object intent,int requestCode, int resultCode){
        System.out.println("facebook sdk onActivityResult enter");
        if (callbackManager != null) {
            callbackManager.onActivityResult(requestCode, resultCode, (Intent)intent);
        }
    }
}


