package com.drowgames.telecompay;
import android.util.Log;
import android.app.Activity;
import cn.egame.terminal.paysdk.EgameExitListener;
public class TelecomOffLineExitGameListener implements EgameExitListener{
    private long m_ptr;
    private Activity m_activity;
    public TelecomOffLineExitGameListener(Activity activity,long ptr) {
    	m_activity = activity;
        m_ptr = ptr;
    }

    public void destory() {
        m_ptr = 0;
    }
    
    @Override
    public void exit() {
      m_activity.finish();
    }

    @Override
    public void cancel() {
      
    }
	

    private void notifyResult(final int result) {
        Log.e("drow", "TelecomOffLineExitGameListener: notifyResult: result=" + result);
        nativeNotifyResult(m_ptr, result);
    }

    private native void nativeNotifyResult(long ptr, int result);
}

