package com.drowgames.chuangku;

import android.app.Activity;
import android.util.Log;
import android.content.Intent;
import org.json.JSONObject;
import com.ck.sdk.CKSDK;
import com.ck.sdk.PayParams;
import com.ck.sdk.interfaces.ExitIAPListener;
import com.ck.sdk.plugin.CKUser;

public class ChuangkuPayManip {
    static long _ptr;
    static Activity m_activity;
    static ChuangkuOffLineListener mSmsListener;
    private static final String TAG = "ChuangkuPayManip";

    public static void init(final Activity activity, final long ptr) {
        activity.runOnUiThread(new Runnable() {
                public void run() {
                    m_activity = activity;
                    _ptr = ptr;
                    mSmsListener = new ChuangkuOffLineListener(m_activity,_ptr);
                    CKSDK.getInstance().setSDKListener(mSmsListener);
                    CKSDK.getInstance().init(m_activity);
                    CKSDK.getInstance().getChannelNo();
                    onStart();
                }
            });
    }
    
	public static void fini() {
        _ptr = 0;
        onDestroy();
    }

    public static  void startPay(final String waresid,final int fee,final String itemName,final String itemDesc) {
        m_activity.runOnUiThread(new Runnable() {
            //@Override
            public void run() {     
                // productId: 支付付费索引编号(这个不要直接传计费点，请cp对每个道具进行编号，整理成表发给创酷技术人员，这里productId传入编号就行，创酷会根据配置的数据去找到计费点进行传入支付)
                PayParams params = new PayParams();
                params.setProductId(waresid);
                params.setPrice(fee);//金额，分为单位
                params.setProductName(itemName);//产品名称
                params.setProductDesc(itemDesc);//产品描述，（如果没有特别描述可以用产品名称代替）
                mSmsListener.StartIap(m_activity, params);         
            }
        });

    }

    public static int getIsMoreGames() {
        Boolean isEgamePlatform = CKSDK.getInstance().isEgamePlatform();
        return isEgamePlatform ? 1:0;
    }


	public static void onShowMoreGamesPage() {
        Log.e(TAG, "onShowMoreGamesPage enter");

        Boolean isEgamePlatform = CKSDK.getInstance().isEgamePlatform();

        if(isEgamePlatform){
            Log.e(TAG, "onShowMoreGamesPage mare game support");

            CKSDK.getInstance().moreGame();
            System.out.println("chuangku sdk onMoreGames enter");
            return;
        }
            Log.e(TAG, "onShowMoreGamesPage mare game not support");
    }

    public static void onShowExitGamePage(){
        System.out.println("chuangku sdk exitGame enter");

        CKSDK.getInstance().exitGame(new ExitIAPListener() {
            
            @Override
            public void showExit() {
                //游戏在这里调用退出界面，一定要实现，因为当渠道没有提供退出方法接口时，我们sdk抽象层会回调这个方法
                mSmsListener.notifySupportExitResult(0);
            }

            @Override
            public void onFinish() {
                // TODO Auto-generated method stub
                CKSDK.getInstance().release();
                m_activity.finish();
                System.exit(0);
            }

            @Override
            public void onCancle() {
                // TODO Auto-generated method stub
            }
        });
    }

    public static String getAdditionAttr() {
        JSONObject parse= CKSDK.getInstance().getGamePersonalCfg();
        if(parse == null) {
            Log.e(TAG, "getAdditionAttr parse == null");
            return "";
        } 

        try {
            Log.e(TAG, "getAdditionAttr" + parse.toString());
            String condition = parse.toString();
            return condition;
          
        } catch (Exception e) {
            Log.e(TAG, "getAdditionAttr error");
            return "";
        }
    }

    public static void onCallAction(String action_name,int level,String level_name,boolean is_success) {
    	System.out.println("chuangku sdk onCallAction enter");
    	if ("more-game".equals(action_name)) {
    		onShowMoreGamesPage();
        }
    	else if("exit".equals(action_name)){
    		onShowExitGamePage();
    	}
    	else if("to_level".equals(action_name))
    	{
    		onToLevel(level,level_name);
    	}
    	else if("exit_level".equals(action_name))
    	{
    		onExitLevel(level,level_name,is_success);
    	}
       	else if("toStore".equals(action_name))
    	{
       		onToStore();
    	}
       	else if("exitStore".equals(action_name))
    	{
       		onExitStore();
    	}
       	else if("toSettlement".equals(action_name))
    	{
       		onToSettlement();
    	}
       	else if("exitSettlement".equals(action_name))
    	{
       		onExitSettlement();
    	}
       	else if("toMainView".equals(action_name))
    	{
       		onToMainView();
    	}
       	else if("exitMainView".equals(action_name))
    	{
       		onExitMainView();
    	}
       	else
       	{
       	 	System.out.println("chuangku sdk onCallAction not function call");
       	}
    }

    public static void onToLevel(int level,String level_name) {
        Log.e(TAG, "level , level_name =" + level + level_name);
        System.out.println("chuangku sdk toLevel enter");
        CKUser.getInstance().toLevel(level,level_name);
    }

    public static void onExitLevel(int level,String level_name,boolean is_success) {
        Log.e(TAG, "level , level_name , is_success =" + level + level_name + is_success);
        System.out.println("chuangku sdk exitLevel enter");
        CKUser.getInstance().exitLevel(level,level_name,is_success);
    }

    public static void onToStore() {
        System.out.println("chuangku sdk toStore enter");
        CKUser.getInstance().toStore();
    }

    public static void onExitStore() {
        System.out.println("chuangku sdk exitStore enter");
        CKUser.getInstance().exitStore();
    }

    public static void onToSettlement() {
        System.out.println("chuangku sdk toSettlement enter");
        CKUser.getInstance().toSettlement();
    }

    public static void onExitSettlement () {
        System.out.println("chuangku sdk exitSettlement enter");
        CKUser.getInstance().exitSettlement ();
    }

    public static void onToMainView() {
        System.out.println("chuangku sdk toMainView  enter");
        CKUser.getInstance().toMainView ();
    }

    public static void onExitMainView () {
        System.out.println("chuangku sdk exitMainView  enter");
        CKUser.getInstance().exitMainView ();
    }

    public static void onPause() {
        System.out.println("chuangku sdk onPause enter");
        CKSDK.getInstance().onPause();
    }

    public static void onSuspend() {
        System.out.println("chuangku sdk onStop enter");
        onPause();
        CKSDK.getInstance().onStop();
    }

    public static void onResume() {
        System.out.println("chuangku sdk onResume enter");
        onRestart();
        CKSDK.getInstance().onResume();
    }

    public static void onStart() {
        System.out.println("chuangku sdk onStart enter");
        CKSDK.getInstance().onStart();
    }

    public static void onRestart() {
        System.out.println("chuangku sdk onRestart enter");
        CKSDK.getInstance().onRestart();
    }
    
    public static void onDestroy() {
        System.out.println("chuangku sdk onDestroy enter");
        CKSDK.getInstance().onDestroy();
    }

    public static void onActivityResult(Object intent,int requestCode, int resultCode){
        System.out.println("chuangku sdk onActivityResult enter");
        CKSDK.getInstance().onActivityResult(requestCode, resultCode, (Intent)intent);
    }

     public static void onNewIntent(Object intent){
        System.out.println("chuangku sdk onNewIntent enter");
        CKSDK.getInstance().onNewIntent((Intent)intent);
    }
};
