package com.drowgames.unicompay;
import android.content.Context;
import android.content.Intent;
import android.app.Activity;
import android.util.Log;
import com.unicom.dcLoader.Utils;
import com.unicom.dcLoader.Utils.UnipayPayResultListener;
import android.os.Handler;
import android.os.Looper;
public class UnicomPayManip {
    static long _ptr;
    private static Handler mHandler = new Handler(Looper.getMainLooper());
    private static UnicomOffLinePayListener offLineOffLineListener;
    
	private static UnipayPayResultListener offLineInitListener = new UnipayPayResultListener() {
		@Override
		public void PayResult(String arg0, int arg1, int arg2, String arg3) {}
	};
	
    public static void init(final Activity activity, long ptr) {
        _ptr = ptr;
        // mHandler.post(new Runnable() {
        //     Utils.getInstances().initPayContext(activity, offLineInitListener);
        //     boolean isinit= Utils.getInstances().isInit();
        //     if(isinit)
        //     {
        //         Log.d("  xxxx","xxxxxxxxxxxxxxxxxxxxtrue");
        //     }else
        //     {
        //         Log.d("  xxxx","xxxxxxxxxxxxxxxxfalse");
        //     }
        // });

        mHandler.post(new Runnable() {
        public void run() {
            Utils.getInstances().initPayContext(activity, offLineInitListener);
            boolean isinit= Utils.getInstances().isInit();
            if(isinit)
            {
                Log.d("  xxxx","xxxxxxxxxxxxxxxxxxxxtrue");
            }else
            {
                Log.d("  xxxx","xxxxxxxxxxxxxxxxfalse");
            }
            
          }
        });
    }
    
	public static void fini() {
        _ptr = 0;
    }

    public static  void startPayOffline(Activity activity ,String waresid) {
    	offLineOffLineListener = new UnicomOffLinePayListener(_ptr);
    	Utils.getInstances().pay(activity, waresid, offLineOffLineListener);
    }
}


