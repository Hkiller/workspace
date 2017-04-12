package com.drowgames.umeng;
import android.util.Log;
import android.app.Activity;
import android.content.Context;
import com.umeng.analytics.game.UMGameAgent;

public class UmengManip {
	public static void init(final Context context, final boolean debugMode, final String appkey, final String channelId) {

        ((Activity)context).runOnUiThread(new Runnable() {
                public void run() {
                    UMGameAgent.startWithConfigure(new UMGameAgent.UMAnalyticsConfig(context, appkey, channelId, UMGameAgent.EScenarioType.E_UM_GAME, true));
                    UMGameAgent.setDebugMode(debugMode);
                    UMGameAgent.init(context);
                    UMGameAgent.setSessionContinueMillis(6000);
                }
            });
    }
}
