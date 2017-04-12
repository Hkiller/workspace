package com.drowgames.device;
import java.util.UUID;
import java.util.Locale;
import java.util.regex.Pattern;
import java.io.InputStream;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.FileFilter;
import java.io.File;
import android.content.Context;
import android.content.Intent;
import android.text.TextUtils;
import android.os.Bundle;
import android.os.Build;
import android.os.Environment;
import android.util.Log;
import android.net.Uri;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.view.Display;

public class Device {
    private static long _ptr;

    public static void init(long ptr) {
        _ptr = ptr;
    }
    
    public static void startInstall(Context context, String apk) {
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setDataAndType(Uri.parse("file://" + apk), "application/vnd.android.package-archive");
        context.startActivity(intent);
    }

	public static String getDeviceID(Context context) {
        // try {
        //     TelephonyManager tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        //     String device_id = tm.getDeviceId();
        //     if(!TextUtils.isEmpty(device_id)) return device_id;
        // }
        // catch (Exception e) {
        //     e.printStackTrace();
        // }

        // try {
        //     WifiManager wifi = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        //     String device_id = wifi.getConnectionInfo().getMacAddress();
        //     if(!TextUtils.isEmpty(device_id)) return device_id;
        // }
        // catch (Exception e) {
        //     e.printStackTrace();
        // }
        
        //return Settings.Secure.getString(context.getContentResolver(), Settings.Secure.ANDROID_ID);
        return getUniquePsuedoID();
	}

	public static String getLanguage() {
        String tmpLanguage =  "cn";
        try{
            tmpLanguage = Locale.getDefault().getLanguage();
            if(tmpLanguage.equals("zh")) {
                String tmpContory = Locale.getDefault().getCountry();
                android.util.Log.d("getLanguage", "tmpContory=" + tmpContory);
                if (tmpContory.equals("CN") || tmpContory.equals("cn") || tmpContory.equals("Cn")) {
                    tmpLanguage = "cn";
                }
                else{
                    tmpLanguage = "en";
                }
            }
            else if(tmpLanguage.equals("en") || tmpLanguage.equals("EN") || tmpLanguage.equals("En")) {
            	tmpLanguage = "en";
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        //Log.d("=====1======", tmpLanguage);
        return tmpLanguage;
	}

    public static String getUniquePsuedoID() {
       String serial = null;

       String m_szDevIDShort = "35" + 
            Build.BOARD.length()%10+ Build.BRAND.length()%10 + 

            Build.CPU_ABI.length()%10 + Build.DEVICE.length()%10 + 

            Build.DISPLAY.length()%10 + Build.HOST.length()%10 + 

            Build.ID.length()%10 + Build.MANUFACTURER.length()%10 + 

            Build.MODEL.length()%10 + Build.PRODUCT.length()%10 + 

            Build.TAGS.length()%10 + Build.TYPE.length()%10 + 

            Build.USER.length()%10 ; //13 位

        try {
            serial = android.os.Build.class.getField("SERIAL").get(null).toString();
           //API>=9 使用serial号
            return new UUID(m_szDevIDShort.hashCode(), serial.hashCode()).toString();
        } catch (Exception exception) {
            //serial需要一个初始化
            serial = "serial"; // 随便一个初始化
        }
        //使用硬件信息拼凑出来的15位号码
        return new UUID(m_szDevIDShort.hashCode(), serial.hashCode()).toString();
    }

    public static String getDeviceModel() {
        return Build.MODEL;
    }

    public static int getCpuFreq() {
        String result = "";
        ProcessBuilder cmd;

        try {
            String[] args = { "/system/bin/cat",
                            "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq" };

            cmd = new ProcessBuilder(args);

            Process process = cmd.start();

            InputStream in = process.getInputStream();
            byte[] re = new byte[24];
            while (in.read(re) != -1) {
                    result = result + new String(re);
            }
            in.close();
        } catch (IOException ex) {

            ex.printStackTrace();
            return 800000;
        }

        result = result.trim();
        try {
            return Integer.parseInt(result);
        }
        catch (Exception e) {
            e.printStackTrace();
            return 800000;
        }

    }

    public static long getTotalMemoryKB() {
        String str1 = "/proc/meminfo";// 系统内存信息文件   
        String str2;  
        String[] arrayOfString;  
        long initial_memory = 0;  
    
        try {  
            FileReader localFileReader = new FileReader(str1);  
            BufferedReader localBufferedReader = new BufferedReader(localFileReader, 8192);  
            str2 = localBufferedReader.readLine();// 读取meminfo第一行，系统总内存大小   
    
            arrayOfString = str2.split("\\s+");  
            for (String num : arrayOfString) {  
                Log.i(str2, num + "\t");  
            }  
            initial_memory = Integer.valueOf(arrayOfString[1]).intValue();
            localBufferedReader.close();  
    
        } catch (Exception e) {
            return 256 * 1024;
        }
        return initial_memory;// Byte转换为KB或者MB，内存大小规格化   
    }
    
    public static int getCoresNum() {
        //Private Class to display only CPU devices in the directory listing 
        class CpuFilter implements FileFilter { 
            @Override 
            public boolean accept(File pathname) { 
                //Check if filename is "cpu", followed by a single digit number 
                if(Pattern.matches("cpu[0-9]", pathname.getName())) { 
                    return true; 
                } 
                return false; 
            } 
        } 

        try { 
            //Get directory containing CPU info 
            File dir = new File("/sys/devices/system/cpu/"); 
            //Filter to only list the devices we care about 
            File[] files = dir.listFiles(new CpuFilter()); 
            //Return the number of cores (virtual CPU devices)
            return files.length; 
        }
        catch(Exception e) { 
            //Default to return 1 core 
            return 1; 
        } 
    }
    
    // <macro name="appsvr_device_network_none" value="1"/>
    // <macro name="appsvr_device_network_wifi" value="2"/>
    // <macro name="appsvr_device_network_wwan" value="3"/>
    public static int getNetworkState(Context context) {
        ConnectivityManager manager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);  
        NetworkInfo mobileInfo = manager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        NetworkInfo wifiInfo = manager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        NetworkInfo activeInfo = manager.getActiveNetworkInfo();

        if (activeInfo == null || !activeInfo.isConnected()) {
            return 1;
        }
        else {
            if (activeInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                return 2;
            }
            else {
                return 3;
            }
        }
    }

    public static void notifyNetworkStateChanged() {
        if (_ptr != 0) {
            onNetworkStateChanged(_ptr);
        }
    }

    public static native void onNetworkStateChanged(long ptr);
}
