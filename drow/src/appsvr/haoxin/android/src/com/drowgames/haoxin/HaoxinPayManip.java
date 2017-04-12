package com.drowgames.haoxin;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import com.zwmobi4096.sdk.Sdk;
import org.json.JSONObject;
import com.zwmobi4096.sdk.debug.debug;
import com.drowgames.helper.DrowActivity;;
public class HaoxinPayManip {
    public static final String SDK_PAYEND_ACTION = "com.drowgames.telecompay";
    static long _ptr;
    static Activity m_activity;
    public static void init(final Activity activity, final long ptr) {
        activity.runOnUiThread(new Runnable() {
                public void run() {
                    m_activity = activity;
                    _ptr = ptr;
                    Sdk.init(m_activity);
                    Sdk.registerHandler(new HaoxinOffLineListener(m_activity,_ptr));
                }
            });
    }
    
    public static void fini() {
        _ptr = 0;
    }

    public static  void startPayOffline(String waresid,int fee,String itemName,String itemDesc) {
        Sdk.getSdk().pay(waresid,fee, SDK_PAYEND_ACTION, itemName, itemDesc);
    }
    
    public static void viewMoreGames() {
        try {
            JSONObject j = new JSONObject();
            j.put("method","more_game");
            j.put("action", SDK_PAYEND_ACTION);
            Sdk.getSdk().req(j.toString());
        }
        catch (Exception e) {
            System.out.println("installapk found Exception" + e.toString());
        }
    }
    
    public static void exitGame() {
        try {
            JSONObject j = new JSONObject();
            j.put("method","exit_game");
            j.put("action", SDK_PAYEND_ACTION);
            Sdk.getSdk().req(j.toString());
        } catch (Exception e) {
            System.out.println("installapk found Exception" + e.toString());
        }
        // EgamePay.exit(m_activity, new TelecomOffLineExitGameListener(m_activity,_ptr));
    }

    public static void isSupportExitGame() {
        // HaoxinOffLineListener.setActionName("exit_game");
        // try {
        //        JSONObject j = new JSONObject();
                
        //                j.put("method","get_support");
        //                j.put("action", SDK_PAYEND_ACTION);
        //                Sdk.getSdk().req(j.toString());
        //        } catch (Exception e) {
                    
        //        }
        // EgamePay.exit(m_activity, new TelecomOffLineExitGameListener(m_activity,_ptr));
    }

    public static void isSupportMoreGame() {
        // HaoxinOffLineListener.setActionName("more_game");
        //  try {
        //     JSONObject j = new JSONObject();
            
        //             // j.put("method","get_support");
        //             // j.put("action", SDK_PAYEND_ACTION);
        //             // Sdk.getSdk().req(j.toString());
        //     } catch (Exception e) {
                
        //     }
        // EgamePay.exit(m_activity, new TelecomOffLineExitGameListener(m_activity,_ptr));
    }

    public static void getPayScreen() {
        try {
            JSONObject j = new JSONObject();
        
            j.put("method","get_config_new");
            j.put("action", SDK_PAYEND_ACTION);
            Sdk.getSdk().req(j.toString());
        } catch (Exception e) {
            
        }
        // EgamePay.exit(m_activity, new TelecomOffLineExitGameListener(m_activity,_ptr));
    }
    public static void pause_game() {
        try {
            JSONObject j = new JSONObject();
             debug.out("lllllll sdk pause_game");
        
            j.put("method","pause_game");
            j.put("action", SDK_PAYEND_ACTION);
            Sdk.getSdk().req(j.toString());
        } catch (Exception e) {
            
        }
        // EgamePay.exit(m_activity, new TelecomOffLineExitGameListener(m_activity,_ptr));
    }

    public static void onSuspend() {
        debug.out("lllllll sdk onSuspend");
        //UMGameAgent.onResume(this);
        Sdk.getSdk().onPause();
    }

    public static void onResume() {
        debug.out("lllllll sdk onResume");
        //UMGameAgent.onResume(this);
        Sdk.getSdk().onResume();
    }

    public static void onActivityResult(Object intent,int requestCode, int resultCode){
        debug.out("lllllll sdk onActivityResult");
        Sdk.onActivityResult(requestCode, resultCode, (Intent)intent); //!!!必须放在onActivityResult中
    }

     public static void onNewIntent(Object intent){
        debug.out("lllllll sdk onNewIntent");  
        Sdk.getSdk().onNewIntent((Intent)intent);
    }

};
