package com.drowgames.cmccpay;
import android.content.Context;
import android.app.Activity;
import android.util.Log;
import cn.cmgame.billing.api.GameInterface;

public class CmccPayManip {
    static long _ptr;

    private static CmccOffLineExitGameListener exitOffLineListener;
    
    public static void init(final Activity activity, long ptr) {
        _ptr = ptr;
        GameInterface.initializeApp(activity);
    }
    
	public static void fini() {
        _ptr = 0;
    }

    public static  void startPayOffline(Activity activity ,String waresid) {
    	GameInterface.doBilling(activity, true, true, waresid, null, new CmccOffLinePayListener(_ptr));
    }
    
	public static void viewMoreGames(Activity activity) {
	    GameInterface.viewMoreGames(activity);     
    }
	
	public static void exitGame(Activity activity) {
		// 移动退出接口，含确认退出UI
		// 如果外放渠道（非移动自有渠道）限制不允许包含移动退出UI，可用exitApp接口（无UI退出）
	    GameInterface.exit(activity, new CmccOffLineExitGameListener(activity,_ptr));
	}
};