package com.drowgames.iapppay;

import android.util.Log;
import android.app.Activity;
import com.iapppay.interfaces.callback.IPayResultCallback;
import com.iapppay.sdk.main.IAppPay;
import com.iapppay.sdk.main.IAppPayOrderUtils;

public class IAppPayUtils {
    private static String s_appid;
	private static String s_appv_key;
	public static String s_platp_key;
    private static IAppPayListener s_listener;
    
    public static void init(Activity activity, long obj_ptr, String appid, String appv_key, String platp_key) {
		IAppPay.init(activity, IAppPay.PORTRAIT, appid);
        s_appid = appid;
        s_appv_key = appv_key;
        s_platp_key = platp_key;
        s_listener = new IAppPayListener(obj_ptr);
    }

    public static void fini() {
        s_appid = null;
        s_appv_key = null;
        s_platp_key = null;
        s_listener.destory();
        s_listener = null;
    }
    
	public static void startPay(Activity activity, String appuserid, int waresid, String waresname, float price, String cporderid) {
        final String transData = getTransdata(appuserid, waresid, waresname, price, cporderid);
        final Activity activity_s = activity;
        IAppPay.startPay(activity_s, transData, s_listener);
	}
    
	private static String getTransdata(String appuserid, int waresid, String waresname, float price, String cporderid) {
		IAppPayOrderUtils orderUtils = new IAppPayOrderUtils();
		orderUtils.setAppid(s_appid);
		orderUtils.setWaresid(waresid);
		orderUtils.setCporderid(cporderid);
		orderUtils.setAppuserid(appuserid);
		orderUtils.setPrice(price);
		orderUtils.setWaresname(waresname);
		return orderUtils.getTransdata(s_appv_key);
	}
}

