package com.drowgames.weixin;
import android.util.Log;
import android.app.Activity;
import com.tencent.mm.sdk.openapi.IWXAPI;
import com.tencent.mm.sdk.openapi.WXAPIFactory;
import com.tencent.mm.sdk.modelmsg.SendAuth;

public class WeixinManip {
    public static Activity mActivity;
	public static IWXAPI mApi;
    public static long m_ptr;
    
	public static void init(long ptr, final Activity activity, final String appid) {
        mActivity = activity;
        m_ptr=ptr;
        mApi = WXAPIFactory.createWXAPI(mActivity,appid, true);
        mApi.registerApp(appid);    	
    }

    public static void fini() {
        m_ptr = null;
        mApi = null;
        mActivity  = null;
    }

	public static void login(String scope, final String state) { 
	    if(!mApi.isWXAppInstalled()) {
	    	mActivity.runOnUiThread(new Runnable() {
                    public void run() {
                        nativeNotifyResult(m_ptr, "", state, 9999, "");
                    }
                });
	        return;
	    }
		
        final SendAuth.Req req = new SendAuth.Req();
        req.scope = scope;
        req.state = state;
        mApi.sendReq(req);
    }

	public native static void nativeNotifyResult(long ptr, String access_token, String state, int error, String errormsg);
}
