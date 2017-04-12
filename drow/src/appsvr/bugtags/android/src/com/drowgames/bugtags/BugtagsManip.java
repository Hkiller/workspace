package com.drowgames.bugtags;

import android.util.Log;
import android.app.Activity;
import com.bugtags.library.Bugtags;
import com.bugtags.library.BugtagsOptions;

public class BugtagsManip {
    public static void init(Activity activity, String appid, int model_e) {
        BugtagsOptions options = new BugtagsOptions.Builder().
            trackingLocation(true).//是否获取位置，默认 true
            trackingCrashLog(true).//是否收集crash，默认 true
            trackingConsoleLog(true).//是否收集console log，默认 true
            trackingUserSteps(true).//是否收集用户操作步骤，默认 true
            trackingNetworkURLFilter("(.*)").//自定义网络请求跟踪的 url 规则，默认 null
            build();
        Bugtags.start(appid, activity.getApplication(), model_e);
    }

    public static void onResume(Activity activity) {
        Bugtags.onResume(activity);
    }

    public static void onPause(Activity activity) {
        Bugtags.onPause(activity);
    }
}

