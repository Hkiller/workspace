package com.drowgames.notify;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

public class DrowDeviceNotifyNotification {
    public static void doNotify(Context paramContext, String mainClassName, String ticker, String title, String text, int id)
    {
        int icon = paramContext.getResources().getIdentifier("icon", "drawable", paramContext.getPackageName());
        
        NotificationManager localNotificationManager = (NotificationManager)paramContext.getSystemService("notification");
        NotificationCompat.Builder localBuilder = new NotificationCompat.Builder(paramContext);
        localBuilder.setSmallIcon(icon);
        localBuilder.setTicker(ticker);
        localBuilder.setContentTitle(title);
        localBuilder.setContentText(text);
        localBuilder.setAutoCancel(true);
		Log.v("DrowGames",mainClassName);
		
        try
        {
            Intent localIntent = new Intent(paramContext, Class.forName(mainClassName));
            localIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            localIntent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
            localBuilder.setContentIntent(PendingIntent.getActivity(paramContext, 0, localIntent, PendingIntent.FLAG_ONE_SHOT));
            Notification notfi =localBuilder.build();
            notfi.icon = icon;
            notfi.defaults|=Notification.DEFAULT_ALL;
            //notfi.flags |= Notification.FLAG_ONGOING_EVENT; // 将此通知放到通知栏的"Ongoing"即"正在运行"组中
            //notfi.flags |= Notification.FLAG_AUTO_CANCEL; // 表明在点击了通知栏中的"清除通知"后，此通知不清除，经常与FLAG_ONGOING_EVENT一起使用
            //notfi.flags |= Notification.FLAG_SHOW_LIGHTS;
            //DEFAULT_ALL     使用所有默认值，比如声音，震动，闪屏等等
            //DEFAULT_LIGHTS  使用默认闪光提示
            //DEFAULT_SOUNDS  使用默认提示声音
            //DEFAULT_VIBRATE 使用默认手机震动，需加上<uses-permission android:name="android.permission.VIBRATE" />权限
            //叠加效果常量
            //notification.defaults=Notification.DEFAULT_LIGHTS|Notification.DEFAULT_SOUND;
            notfi.ledARGB = Color.BLUE;
            notfi.ledOnMS =50; //闪光时间，毫秒
		
            notfi.ledARGB = 0xff00ff00;
            notfi.ledOnMS = 300;
            notfi.ledOffMS = 1000;
            notfi.flags |= Notification.FLAG_SHOW_LIGHTS;
		
            localNotificationManager.cancel(id);
            // 把Notification传递给NotificationManager
            localNotificationManager.notify(id, notfi);
        }
        catch (ClassNotFoundException localClassNotFoundException)
        {
        	localClassNotFoundException.printStackTrace();
        }
    }
}
