package com.drowgames.yomobads;
import android.util.Log;
import android.app.Activity;
import java.util.Map;
import com.soulgame.sgsdk.tgsdklib.TGSDK;
import com.soulgame.sgsdk.tgsdklib.TGSDKServiceResultCallBack;
import com.soulgame.sgsdk.tgsdklib.ad.ITGADListener;
import com.soulgame.sgsdk.tgsdklib.ad.ITGPreloadListener;
import com.soulgame.sgsdk.tgsdklib.ad.ITGRewardVideoADListener;

public class YomobadsListener {
    private long m_ptr;
    private static Activity m_activity;
    private boolean mInit = false;
    private static final String TAG = "YomobadsListener";

    public YomobadsListener(final Activity activity,final long ptr) {
        m_ptr = ptr;
        m_activity = activity;
        m_activity.runOnUiThread(new Runnable() {
            public void run() {
                TGSDK.initialize(activity,new TGSDKServiceResultCallBack() {
                    @Override
                    public void onFailure(Object arg0, String arg1) {
                        android.util.Log.e("TGSDK", "init fail");
                    }

                    @Override
                    public void onSuccess(Object arg0, Map<String, String> arg1) {
                        android.util.Log.d("TGSDK", "init success");
                    }
                });
                
                TGSDK.preloadAd(new ITGPreloadListener() {
                    @Override
                    public void onPreloadSuccess(String result) {
                        Log.e(TAG, "onPreloadSuccess");

                        // 广告预加载调用成功
                    }

                    @Override
                    public void onPreloadFailed(String scene, String error) {
                        Log.e(TAG, "onPreloadFailed");

                        // 广告预加载调用失败
                    }

                    @Override
                    public void onCPADLoaded(String result) {
                        Log.e(TAG, "onCPADLoaded");

                        // 静态插屏广告已就绪
                    }

                    @Override
                    public void onVideoADLoaded(String result) {
                        Log.e(TAG, "onVideoADLoaded");

                        // 视频广告已就绪
                    }

                });

                TGSDK.setRewardVideoADListener(new ITGRewardVideoADListener() {
                    public void onADAwardSuccess(String result) {
                        Log.e(TAG, "onADAwardSuccess");

                        // 奖励广告条件达成，可以向用户发放奖励
                        notifyAdResult(4);
                    }

                    public void onADAwardFailed(String result, String error) {
                        Log.e(TAG, "onADAwardFailed");

                        // 奖励广告条件未达成，无法向用户发放奖励
                    }

                });

                TGSDK.setADListener(new ITGADListener() {
                    public void onShowSuccess(String result) {
                        Log.e(TAG, "onShowSuccess");

                        // 广告开始播放
                    }

                    public void onShowFailed(String result, String error) {
                        Log.e(TAG, "onShowFailed");

                        // 广告播放失败
                        notifyAdResult(1);
                    }

                    public void onADComplete(String result) {
                        Log.e(TAG, "onADComplete");

                        // 广告播放完成
                    }

                    public void onADClick(String result) {
                        Log.e(TAG, "onADClick");

                        // 用户点击了广告，正在跳转到其他页面
                    }

                    public void onADClose(String result) {
                        Log.e(TAG, "onADClose");
                        
                        // 广告关闭
                    }
                });       
            }     
        });
    }

    public void destory() {
        m_ptr = 0;
    }
    
    private void notifyAdResult(final int result) {
        Log.e(TAG, "yomobads: notifyResult: result=" + result);
        nativeNotifyAdResult(m_ptr, result);
    }

    private native void nativeNotifyAdResult(long ptr, int result);
}

