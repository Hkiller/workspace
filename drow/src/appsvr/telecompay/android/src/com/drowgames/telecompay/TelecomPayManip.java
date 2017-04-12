package com.drowgames.telecompay;

import android.app.Activity;
import android.util.Log;
import java.util.HashMap;
import java.util.Map;
import android.app.AlertDialog.Builder;
import cn.egame.terminal.paysdk.EgamePay;
public class TelecomPayManip {
    static long _ptr;
    static Activity m_activity;
    public static void init(final Activity activity, final long ptr) {
        activity.runOnUiThread(new Runnable() {
            public void run() {
                m_activity = activity;
                _ptr = ptr;
                EgamePay.init(m_activity);
            }
        });
    }
    
	public static void fini() {
        _ptr = 0;
    }

    public static  void startPayOffline(String waresid) {
        HashMap<String, String> payParams = new HashMap<String, String>();
        payParams.put(EgamePay.PAY_PARAMS_KEY_TOOLS_ALIAS, waresid);
        payParams.put(EgamePay.PAY_PARAMS_KEY_PRIORITY, "sms");
        EgamePay.pay(m_activity, payParams, new TelecomOffLinePayListener(m_activity,_ptr));
    }
    
	public static void viewMoreGames() {
	    EgamePay.moreGame(m_activity);   
    }
	
	public static void exitGame() {
        EgamePay.exit(m_activity, new TelecomOffLineExitGameListener(m_activity,_ptr));
	}
};