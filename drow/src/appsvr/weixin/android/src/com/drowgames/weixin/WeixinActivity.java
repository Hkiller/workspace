package com.drowgames.weixin;
import com.tencent.mm.sdk.modelmsg.SendAuth;
import com.tencent.mm.sdk.modelbase.BaseReq;
import com.tencent.mm.sdk.modelbase.BaseResp;
import com.tencent.mm.sdk.openapi.IWXAPI;
import com.tencent.mm.sdk.openapi.IWXAPIEventHandler;
import com.tencent.mm.sdk.openapi.WXAPIFactory;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class WeixinActivity extends Activity implements IWXAPIEventHandler {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        WeixinManip.mApi.handleIntent(getIntent(), this);
    }
    
    @Override
    protected void onNewIntent(Intent intent) {
    	super.onNewIntent(intent);
    	setIntent(intent);
    	WeixinManip.mApi.handleIntent(intent, this);
    }

	@Override
	public void onReq(BaseReq arg0) {
	}

	@Override
	public void onResp(BaseResp arg0) {
		switch (arg0.errCode){
		case BaseResp.ErrCode.ERR_OK:
			WeixinManip.nativeNotifyResult(WeixinManip.m_ptr, ((SendAuth.Resp)arg0).code, "", 0, "");
			break;
        default:
    		Log.e("drow", "WeixinActivity: onResp: receiv error response: errno=" + arg0.errCode + "error-msg=" + arg0.errStr);
			WeixinManip.nativeNotifyResult(WeixinManip.m_ptr, null, null, arg0.errCode, arg0.errStr == null ? "" : arg0.errStr);        	
            break;
		}
        
		finish();
	}
}
