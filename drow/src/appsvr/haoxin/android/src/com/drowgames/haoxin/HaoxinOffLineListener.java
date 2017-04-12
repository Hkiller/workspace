package com.drowgames.haoxin;
import android.util.Log;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import com.zwmobi4096.sdk.debug.debug;
import org.json.JSONObject;

public class HaoxinOffLineListener extends Handler{
    private long m_ptr;
    private static Activity m_activity;
    private static String m_supportActionName;

    public static void setActionName(String actionName){
        m_supportActionName = actionName;
    }

    public HaoxinOffLineListener(Activity activity,long ptr) {
        m_ptr = ptr;
        m_activity = activity;
        nativeNotifyGetPayScreenResult(m_ptr,"A"/*appsvr_sdk_payscreen_A*/);//A界面1
    }

    public void destory() {
        m_ptr = 0;
    }
    
    @Override
    public void handleMessage(Message msg) {
            if (msg.what == 0) {
                    //成功的消息
                    String str = (String)(msg.obj);
                    debug.out("msg.what == 0" + str);
                    try {
                            JSONObject parse= new JSONObject(str);
                            debug.out("msg.what == 0 method = " + parse.getString("method"));
                            if ("pay_result".equals(parse.getString("method"))) {// 支付成功
                                    Log.i("drow", "haoxin: pay success");
                                    notifyResult(0/*appsvr_payment_success*/, 0, "");
                            }

                            if ("start_app".equals(parse.getString("method"))) {// 支付成功
                                    HaoxinPayManip.getPayScreen();
                            }

                            if ("get_config_new".equals(parse.getString("method"))) {//获得支付界面类型 A B
                                String mode = parse.getJSONObject("data").optString("payscreen", "A");
                                if(mode.equals("A")){
                                    nativeNotifyGetPayScreenResult(m_ptr,"A"/*appsvr_sdk_payscreen_A*/);//A界面1
                                }
                                else{
                                    nativeNotifyGetPayScreenResult(m_ptr,"B"/*appsvr_sdk_payscreen_B*/);//B界面2
                                }        
                            }  

                            if ("get_support".equals(parse.getString("method"))) {//是否支持    
                                String str1 = parse.getJSONArray("data").toString();         	
                                int pos = str1.lastIndexOf("exit_game");
                                if(pos<0){
                                    nativeNotifyExitGameSupportResult(m_ptr,0/*appsvr_not_support*/);//不支持
                                }
                                else{
                                    nativeNotifyExitGameSupportResult(m_ptr,1/*appsvr_support*/);//支持
                                }   

                                pos = str1.lastIndexOf("more_game");
                                if(pos<0){
                                    nativeNotifyMoreGameSupportResult(m_ptr,0/*appsvr_not_support*/);//不支持
                                }
                                else{
                                    nativeNotifyMoreGameSupportResult(m_ptr,1/*appsvr_support*/);//支持
                                }  

                            }  

                    } catch (Exception e) {
                            debug.out("sdk on receive "+e);
                    }
                    
            } else {
            		//失败的消息
                    String str = (String)(msg.obj);
                    try {
                        	JSONObject parse= new JSONObject(str);
                     
                        	if ("pay_result".equals(parse.getString("method"))) {
                        		String ret = parse.getJSONObject("data").getString("ret");
                                Log.i("drow", "haoxin: pay fail");
                                notifyResult(2/*appsvr_payment_failed*/, Integer.parseInt(ret), "");
                        	}
                        	
                            if ("exit_game".equals(parse.getString("method"))) {// 退出游戏 点击取消
                            	//nativeNotifyExitCancelResult(m_ptr);
                            	
                            }
                    	} catch (Exception e) {
                    		debug.out("sdk on receive"+e);
                    	}
            }
    }
    
    private void notifySupportResult(final int result) {
        Log.i("drow", "haoxin: notifySupportResult: result=" + result + "m_supportActionName=" + m_supportActionName);
    	if(m_supportActionName.lastIndexOf("exit_game")>=0)
    	{
             //Log.i("drow", "notifySupportResult: m_supportActionName");
            nativeNotifyExitGameSupportResult(m_ptr, result);
    	}
    	else
    	{
            nativeNotifyMoreGameSupportResult(m_ptr, result);
    	}
    }
    
    private void notifyResult(final int result, final int service_result, final String error_msg) {
        Log.e("drow", "haoxin: notifyResult: result=" + result + ", service_result=" + service_result + ", error_msg=" + error_msg);
        nativeNotifyPayResult(m_ptr, result, service_result, error_msg);
    }
    
    private native void nativeNotifyExitCancelResult(long ptr);
    private native void nativeNotifyPayResult(long ptr, int result, int service_result, String error_msg);
    private native void nativeNotifyExitGameSupportResult(long ptr, int result);
    private native void nativeNotifyMoreGameSupportResult(long ptr, int result);
    private native void nativeNotifyGetPayScreenResult(long ptr, String payscreen);
}

