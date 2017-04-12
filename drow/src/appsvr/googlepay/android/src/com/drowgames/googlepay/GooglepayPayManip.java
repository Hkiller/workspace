package com.drowgames.googlepay;
import android.app.Activity;
import android.util.Log;
import com.drowgames.helper.DrowActivity;

import java.util.ArrayList;
import java.util.List;

import com.android.vending.billing.IInAppBillingService;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.RemoteException;
import android.content.Context;
import android.content.Intent;
import com.drowgames.googlepay.util.IabHelper;
import com.drowgames.googlepay.util.IabResult;
import com.drowgames.googlepay.util.Inventory;
import com.drowgames.googlepay.util.Purchase;
import com.drowgames.googlepay.util.IabHelper.OnConsumeMultiFinishedListener;
import org.json.JSONException;
import org.json.JSONObject;
import android.os.Handler;
import android.os.Message;

public class GooglepayPayManip {
    static long _ptr;
    static Activity m_activity;
    static final String TAG = "googlepay";
    static final int RC_REQUEST = 10001;
    static IabHelper mHelper;
    static GooglepayOffLineListener googlelistener;
    static IInAppBillingService mService;
    static Purchase m_purchase;
    static boolean m_is_init_success = false;
    public static void init(final Activity activity, final long ptr) {
        activity.runOnUiThread(new Runnable() {
            public void run() {
                m_activity = activity;
                _ptr = ptr;
                String base64EncodedPublicKey = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgKYF0XD1ILPtGHbJwk7zlboD4i4POHufQBZrCQ8wdNML6mRqYcl7qDt4iYrzbIrYZRQ4HAgG/znN84Shoq064ra4k2Q1tRjmiartM7V6T28TDctNkTmDbLAgJ3T8KGKDKFDG5ogfB9+fp60EI4ChDaCFNm9ba9yNR6O0OQl9kRmEt46AORoiNc2TgJqITPhpR9QlaZbNUM7JdTqjJjkQeTjlMwErZsAMZ0jw+gX61HRQYYUZPto1Mtmic1fCvfHx86XvL4HYxgZJh7f3HzsYM8e1GbWVePqhCrERO46rKmjI/DhDfBdvU/uyfuhHYqoXaxNXN2VD+dlsaU2MZ8OVgwIDAQAB";
                mHelper = new IabHelper(activity, base64EncodedPublicKey);
                googlelistener = new GooglepayOffLineListener(ptr);
                mHelper.enableDebugLogging(false);

                Log.d(TAG, "Starting setup.");
                mHelper.startSetup(new IabHelper.OnIabSetupFinishedListener() {
                    public void onIabSetupFinished(IabResult result) {
                        Log.d(TAG, "Setup finished.");

                        if (!result.isSuccess()) {
                            complain("Problem setting up in-app billing: " + result);
                            return;
                        }

                        if (mHelper == null) return;
                        m_is_init_success = true;
                        Log.d(TAG, "Setup successful. Querying inventory.");
                        mHelper.queryInventoryAsync(mGotInventoryListener);

                    }
                });
            }     
        });
    }

    static IabHelper.QueryInventoryFinishedListener mGotInventoryListener = new IabHelper.QueryInventoryFinishedListener() {
        public void onQueryInventoryFinished(IabResult result, Inventory inventory) {
            Log.d(TAG, "Query inventory finished.");

            if (mHelper == null) return;

            if (result.isFailure()) {
                complain("Failed to query inventory: " + result);
                return;
            }

            List<Purchase> all_purchase = inventory.getAllPurchases();
            if (all_purchase != null) {
                Log.d(TAG, "We have gas. Consuming it.");
                mHelper.consumeAsync(all_purchase, null);
                return;
            }
        }
    };

	public static void fini() {
        onDestroy();
        _ptr = 0;
    }

    public static  void startPay(final String waresid,final int fee,final String itemName,final String itemDesc) {
        if(m_is_init_success){
            String payload = "";
            mHelper.launchPurchaseFlow(m_activity,waresid, RC_REQUEST,
            mPurchaseFinishedListener, payload);
        }
        else{
            m_activity.runOnUiThread(new Runnable() {
                public void run() {
                    try {
                        Thread.sleep(1000);
                        googlelistener.notifyResult(6/*appsvr_payment_failed*/,-1, "");
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                }     
            });
        }

    }

    public static void getAllItemInfo(){
        Log.d(TAG, "getAllItemInfo enter.");
        try {
                querySkuDetails();
        } catch (RemoteException e) {
                 // TODO Auto-generated catch block
             e.printStackTrace();
        } catch (JSONException e) {
                 // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public static void querySkuDetails() throws RemoteException, JSONException {
        m_activity.runOnUiThread(new Runnable() {
            public void run() {
            	try {
	                ArrayList<String> skuList = new ArrayList<String> ();
	                skuList.add("2");
	                skuList.add("3");
	                skuList.add("4");
	                skuList.add("5");
	                skuList.add("6");
	                skuList.add("7");
	                skuList.add("8");
	                skuList.add("9");
                    skuList.add("10");	                
	                Bundle querySkus = new Bundle();
	                querySkus.putStringArrayList("ITEM_ID_LIST", skuList);
	                mService = mHelper.mService;
	                if(mHelper.mService==null){
	                	googlelistener.notifyProductInfoResult(null,null);
	                	return;
	                }
	                
	                Bundle skuDetails = mHelper.mService.getSkuDetails(3, m_activity.getPackageName(),"inapp", querySkus);
	                int response = skuDetails.getInt("RESPONSE_CODE");
	                if (response == 0) {
	                    ArrayList<String> responseList
	                      = skuDetails.getStringArrayList("DETAILS_LIST");
	
	                    for (String thisResponse : responseList) {
	                       
	                      JSONObject object = new JSONObject(thisResponse);
	                      String sku = object.getString("productId");
	                      String price = object.getString("price");
	                      googlelistener.notifyProductInfoResult(sku,price);
	                    }
	                    googlelistener.notifyProductInfoResult(null,null);
	                }
	            } catch (RemoteException e) {
	                // TODO Auto-generated catch block
	                //e.printStackTrace();
	            }
	            catch (JSONException e) {
	                // TODO Auto-generated catch block
	                //e.printStackTrace();
	            }
            }
        });   
        
    }
    
    static IabHelper.OnIabPurchaseFinishedListener mPurchaseFinishedListener = new IabHelper.OnIabPurchaseFinishedListener() {
        public void onIabPurchaseFinished(IabResult result, Purchase purchase) {
            Log.d(TAG, "Purchase finished: " + result + ", purchase: " + purchase);

            if (mHelper == null) return;

            if (result.isFailure()) {
                complain("Error purchasing: " + result);
                m_activity.runOnUiThread(new Runnable() {
                        public void run() {
                            try {
                                Thread.sleep(100);
                                googlelistener.notifyResult(2/*appsvr_payment_failed*/,-1, "");
                            } catch (InterruptedException e) {
                                // TODO Auto-generated catch block
                                e.printStackTrace();
                            }
                        }     
                });
                
                return;
            }
            
            m_purchase = purchase;
            m_activity.runOnUiThread(new Runnable() {
                    public void run() {
                        try {
                            googlelistener.notifyResult(0/*appsvr_payment_success*/, 0, "");
                            Thread.sleep(1000);
                            mHelper.consumeAsync(m_purchase, null);
                        } catch (InterruptedException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                    }     
            });
            Log.d(TAG, "Purchase successful.");
        }
    };
 
    public static void onDestroy() {
        System.out.println("googlepay sdk onDestroy enter");
        Log.d(TAG, "Destroying helper.");
        if (mHelper != null) {
            mHelper.dispose();
            mHelper = null;
        }
    }

    public static void onActivityResult(Object intent,int requestCode, int resultCode){
        System.out.println("googlepay sdk onActivityResult enter");
        if (mHelper.handleActivityResult(requestCode, resultCode, (Intent)intent)) {
        	 Log.d(TAG, "onActivityResult handled by IABUtil.");
        }
    }

    static void complain(String message) {
        Log.e(TAG, "**** googlepay Error: " + message);
    }
};
