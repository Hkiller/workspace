package com.drowgames.notify;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import android.annotation.SuppressLint;
import android.app.AlarmManager;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class DrowDeviceNotifyManager {
    public static void addNoticfy(Context instance, String title, String content, int delalt,int key,int repeatTime)
    {
        Log.d("DrowGames","addNoticfy enter:");
        JSONObject j = new JSONObject();
        try {
            j.put("ticker", content);
            j.put("title", title);
            j.put("text", content);
            if(repeatTime <= 0)
            {
                j.put("tag", "once");
            }
            else
            {
                j.put("intervalAtHour", repeatTime);
            }
            long offest = delalt - System.currentTimeMillis()/1000;
            j.put("triggerOffset", offest);
            j.put("id", key);
            j.put("packageName", instance.getPackageName());/*包名注意填 */
            j.put("MainClassName", instance.getClass().getName());
            DrowDeviceNotifyManager.alarmNotify(instance, j.toString());
            
        } catch (JSONException e) {
            Log.d("DrowGames","addNoticfy enter fail:");
            e.printStackTrace();
        }
    }

    @SuppressLint({ "SimpleDateFormat", "DefaultLocale" })
	public static void alarmNotify(Context Context, String jsonString)
    {
        AlarmManager localAlarmManager = (AlarmManager)Context.getSystemService(android.content.Context.ALARM_SERVICE);
        
        long intervalAtMillis = 0;
        long triggerAtMillis = 0;

        int type = AlarmManager.RTC_WAKEUP;
        PendingIntent localPendingIntent;

        try
        {
          JSONObject localJSONObject = new JSONObject(jsonString);
          String packageName = localJSONObject.optString("packageName",Context.getPackageName());
          String ticker = localJSONObject.optString("ticker", "null");
          String title = localJSONObject.optString("title", "null");
          String text = localJSONObject.optString("text", "null");
          String str1 = localJSONObject.optString("tag", "noonce");
          String strClassName = localJSONObject.optString("MainClassName", "null");
          long triggerOffset = localJSONObject.optLong("triggerOffset", 0L);
          long intervalAtHour = localJSONObject.optLong("intervalAtHour", 0);
          int id = localJSONObject.optInt("id", 0);

          triggerOffset *= 1000L;

          triggerAtMillis = System.currentTimeMillis() + triggerOffset;

          Intent localIntent = new Intent("drow_notify_device_receiver");//广播名，时间到了就会发送drow_notify_device_receiver
          Bundle localBundle = new Bundle();
          localBundle.putInt("flag", id);
          localBundle.putString("packageName", packageName);
          localBundle.putString("ticker", ticker);
          localBundle.putString("title", title);
          localBundle.putString("text", text);
          localBundle.putString("MainClassName", strClassName);
          localIntent.putExtras(localBundle);
          localPendingIntent = PendingIntent.getBroadcast(Context, id, localIntent, PendingIntent.FLAG_UPDATE_CURRENT);
          if (str1.equals("once"))
          {
              localAlarmManager.set(type, triggerAtMillis, localPendingIntent);
              
              Log.v("DrowGames","cur_time :" + System.currentTimeMillis());
              Log.v("DrowGames","triggerAtMillis :" + triggerAtMillis);
          }
          else
          {
            
              SimpleDateFormat formart= new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

              intervalAtMillis = 24L * 3600L * 1000L;
              int year = Calendar.getInstance().get(Calendar.YEAR);
              int month = Calendar.getInstance().get(Calendar.MONTH) + 1;
              int day = Calendar.getInstance().get(Calendar.DAY_OF_MONTH);
              int hour = Calendar.getInstance().get(Calendar.HOUR_OF_DAY);
              int minute = Calendar.getInstance().get(Calendar.MINUTE);
              int second = Calendar.getInstance().get(Calendar.SECOND);

              String time_cur = String.format("%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second); //2014-01-04
              String time_next = String.format("%04d-%02d-%02d %02d:%02d:%02d", year, month, day, intervalAtHour, 0, 0); //2014-01-04

              Log.v("DrowGames","time_cur :" + time_cur);
              Log.v("DrowGames","time_next :" + time_next);

              java.util.Date date1 =formart.parse(time_cur.toString());
              java.util.Date date2 =formart.parse(time_next.toString());
              long times_offset= date2.getTime() - date1.getTime();//这样得到的差值是微秒级别
              if(times_offset < 0)
              {
                times_offset = times_offset + 24L * 3600L * 1000L;
              }
              
              Log.v("DrowGames","times_offset :" + times_offset);

              localAlarmManager.setRepeating(type , System.currentTimeMillis() + times_offset, intervalAtMillis, localPendingIntent);
              
              Log.v("DrowGames","setRepeating :" + intervalAtMillis);
          }
        }
        catch (JSONException localJSONException)
        {
        } 
        catch (ParseException e) {
			e.printStackTrace();
		}
    }
    
    public static void cancelNotify(Context paramContext, int paramInt)
      {
        NotificationManager localNotificationManager = (NotificationManager)paramContext.getSystemService("notification");
        localNotificationManager.cancel(paramInt);
        
        AlarmManager localAlarmManager = (AlarmManager)paramContext.getSystemService(android.content.Context.ALARM_SERVICE);
        PendingIntent localPendingIntent = PendingIntent.getBroadcast(paramContext, paramInt, new Intent("drow_notify_device_receiver"), PendingIntent.FLAG_NO_CREATE);
        if (localPendingIntent == null)
          return;
        localAlarmManager.cancel(localPendingIntent);
      }

      public static void cancelNotify(Context paramContext, String paramString)
      {
        AlarmManager localAlarmManager = (AlarmManager)paramContext.getSystemService(android.content.Context.ALARM_SERVICE);
        try
        {
          JSONArray localJSONArray = new JSONObject(paramString).optJSONArray("piids");
          int i = 0;
          if (i >= localJSONArray.length())
            return;
          PendingIntent localPendingIntent = PendingIntent.getBroadcast(paramContext, localJSONArray.getInt(i), new Intent("drow_notify_device_receiver"), PendingIntent.FLAG_NO_CREATE);
          if (localPendingIntent != null)
            localAlarmManager.cancel(localPendingIntent);
          ++i;
        }
        catch (JSONException localJSONException)
        {
          localJSONException.printStackTrace();
        }
      }
}
