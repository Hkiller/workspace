package com.drowgames.notify;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class DrowDeviceNotifyBroadcastReceiver extends BroadcastReceiver
{

    @Override
    public void onReceive(Context context, Intent intent) {

        if(intent.getAction().equals("drow_notify_device_receiver"))
        {
            Log.v("DrowGames","DrowDeviceNotifyService onReceive");
            Bundle localBundle = intent.getExtras();
            String ticker = localBundle.getString("ticker");
            String title = localBundle.getString("title");
            String text = localBundle.getString("text");
            String className = localBundle.getString("MainClassName");
            int id = localBundle.getInt("flag");
            Log.v("DrowGames","DrowDeviceNotifyService onReceive2  " + className);
            DrowDeviceNotifyNotification.doNotify(context, className, ticker, title, text,id);//开始本地推送
        }
    }
} 
