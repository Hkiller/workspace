package com.drowgames.yomobads;

import android.app.Activity;
import android.content.Intent;
import com.soulgame.sgsdk.tgsdklib.TGSDK;

public class YomobadsManip {
    static long _ptr;
    static Activity m_activity;

    static YomobadsListener yomobadsListener;
    public static void init(final Activity activity, final long ptr) {
        TGSDK.setDebugModel(false);
        activity.runOnUiThread(new Runnable() {
                public void run() {
                    m_activity = activity;
                    _ptr = ptr;
                    yomobadsListener = new YomobadsListener(m_activity,_ptr);
                }     
        });
    }

	public static void fini() {
        _ptr = 0;
    }

    public static  void startAdsOpen(final String adsid) {
        m_activity.runOnUiThread(new Runnable() {
            public void run() {
                TGSDK.showAd(m_activity,adsid);
            }     
        });
    }

    public static void onPause() {
        System.out.println("yomobads sdk onPause enter");
        TGSDK.onPause(m_activity);
    }

    public static void onSuspend() {
        onPause();
        System.out.println("yomobads sdk onStop enter");
        TGSDK.onStop(m_activity);
    }

    public static void onResume() {
        System.out.println("yomobads sdk onResume enter");
        TGSDK.onResume(m_activity);
    }

    public static void onStart() {
        System.out.println("yomobads sdk onStart enter");
        TGSDK.onStart(m_activity);
    }
    
    public static void onDestroy() {
        System.out.println("yomobads sdk onDestroy enter");
        TGSDK.onDestroy(m_activity);
    }

    public static void onActivityResult(Object intent,int requestCode, int resultCode){
        System.out.println("yomobads sdk onActivityResult enter");
        TGSDK.onActivityResult(m_activity,requestCode, resultCode, (Intent)intent);
    }
};
