package com.drowgames.cmccpay;
import android.util.Log;
import android.app.Activity;
import cn.cmgame.billing.api.GameInterface.GameExitCallback;

public class CmccOffLineExitGameListener implements GameExitCallback{
    private long m_ptr;
    private Activity m_activity;
    public CmccOffLineExitGameListener(Activity activity,long ptr) {
    	m_activity = activity;
        m_ptr = ptr;
    }

    public void destory() {
        m_ptr = 0;
    }
    
    @Override
    public void onConfirmExit() {
      m_activity.finish();
      System.exit(0);
    }

    @Override
    public void onCancelExit() {
      
    }
	

    private void notifyResult(final int result) {
        Log.e("drow", "CmccOffLineExitGameListener: notifyResult: result=" + result);
        nativeNotifyResult(m_ptr, result);
    }

    private native void nativeNotifyResult(long ptr, int result);
}

