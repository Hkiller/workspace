package com.drowgames.device;
import android.content.Context;
import android.content.Intent;

public class BroadcastReceiver extends android.content.BroadcastReceiver {  
    @Override  
    public void onReceive(Context context, Intent intent) {
        Device.notifyNetworkStateChanged();
    }  //如果无网络连接activeInfo为null  
} 
