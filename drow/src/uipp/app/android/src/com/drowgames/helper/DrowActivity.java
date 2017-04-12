package com.drowgames.helper;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import java.util.Locale;
import java.io.File;
import java.lang.System;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.util.Log;
import android.util.DisplayMetrics;
import android.text.TextUtils;
import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Environment;
import android.view.View;
import android.view.ViewGroup;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;
import android.view.Display;
import android.widget.ImageView;
import android.widget.FrameLayout;
import android.widget.EditText;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.graphics.Point;
import android.content.Context;
import android.content.Intent;
import android.content.ActivityNotFoundException;
import android.provider.Settings;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.os.Build;  
import android.net.wifi.WifiManager;
import android.opengl.GLSurfaceView;
import android.opengl.EGL14;
import android.opengl.GLUtils;

public class DrowActivity extends Activity {
    public native void nativeStop();
    public native void nativeDone();
    
    protected int m_id_logo;

    private SystemUiHelper mUiHelper;
    private FrameLayout mRootLayout;
    private DrowGLSurfaceView mGLView;
    private ImageView m_logo_view;
    private boolean mSuspend = true;
    public boolean mInit = false;
    
    @SuppressLint("NewApi")
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d("drow", "onCreate");
        
        setJavaVM();
        setAndroidApk(this.getApplicationInfo().publicSourceDir);
        setInternalDir(getFilesDir().getAbsolutePath());
        File externalDir = getExternalFilesDir(null);
        if (externalDir != null) setExternalDir(externalDir.getAbsolutePath());
        
        setAssetManager(getAssets());

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN); 

        super.onCreate(savedInstanceState);

        mUiHelper = new SystemUiHelper(this, SystemUiHelper.LEVEL_IMMERSIVE, SystemUiHelper.FLAG_IMMERSIVE_STICKY);
        mUiHelper.hide();

        mRootLayout = new FrameLayout(this);
        mRootLayout.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.FILL_PARENT));

        mGLView = new DrowGLSurfaceView(this);
        mGLView.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.FILL_PARENT));
        mRootLayout.addView(mGLView);

        m_logo_view = new ImageView(this);
        m_logo_view.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        m_logo_view.setImageResource(m_id_logo);
        m_logo_view.setScaleType(ImageView.ScaleType.FIT_XY);
        mRootLayout.addView(m_logo_view);
        
        setContentView(mRootLayout);
    }

    public boolean isInit() {
        return mInit;
    }

    public Point displaySize() {
        WindowManager wm = ((WindowManager)getSystemService(Context.WINDOW_SERVICE));
        Display display = wm.getDefaultDisplay();
        
        Point r = new Point();
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR2) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
                display.getRealSize(r);
            }
            else {
                display.getSize(r);
            }
        }
        else {
            r.x= display.getWidth();
            r.y = display.getHeight();
        }

        return r;
    }
    
    @Override  
    protected void onRestart() {
        Log.d("drow", "DrowActivity: onRestart");

        super.onRestart();
        if (mInit) {
            runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        nativeOnRestart();
                    }
                });
        }
    }
    
    @Override
    protected void onStart() {
        Log.d("drow", "DrowActivity: onStart");

        super.onStart();
        if (mInit) {
            runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        nativeOnStart();
                    }
                });
        }
    }
    
    @Override
    protected void onResume() {
        Log.d("drow", "DrowActivity: onResume");

        super.onResume();
        //mGLView.onResume();

        mUiHelper.hide();
        //Log.d("drow", "xxxxx: call hide");

        mSuspend = false;
        
        if (mInit) {
            runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        nativeOnResume();
                    }
                });
        }
    }
    
    @Override
    protected void onPause() {
        Log.d("drow", "DrowActivity: onPause");

        super.onPause();
        //mGLView.onPause();
         
        mSuspend = true;
        
        if (mInit) {
            runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        nativeOnPause();
                    }
                });
        }
    }
    
    @Override
    protected void onActivityResult(final int requestCode, final int resultCode, final Intent intent){
        Log.d("drow", "DrowActivity: onActivityResult");

        super.onActivityResult(requestCode, resultCode, intent);

        if (mInit) {
            runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        nativeOnActivityResult(intent, requestCode, resultCode);
                    }
                });
        }
    }

    @Override
    public void onNewIntent(final Intent intent) {
        Log.d("drow", "DrowActivity: onNewIntent");

        super.onNewIntent(intent);
        if (mInit) {
            runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        nativeOnNewIntent(intent);
                    }
                });
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        Log.d("drow", "DrowActivity: onSaveInstanceState");

        super.onSaveInstanceState(outState);
        //String str = txt.getText().toString();
       // outState.putString("info", ( ( str.equals(null) ) == true ) ? "empty info" : str );
        Log.d("--- onSaveInstanceState called --- ", outState.toString());
    }
    @Override
    protected void onStop() {
        Log.d("drow", "DrowActivity: onStop");
        super.onStop();
        if (mInit) {
            runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        nativeOnStop();
                    }
                });
        }
    }

    @Override
    protected void onDestroy() {
        Log.d("drow", "DrowActivity onDestroy");
        super.onDestroy();

        if (mInit) {
            runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        nativeDone();
                    }
                });
        }
    }

    @Override
    public void onLowMemory() {
        Log.d("drow", "DrowActivity: onLowMemory");
    }

    public void onWindowFocusChanged(boolean hasFocus) {
        Log.d("drow", "DrowActivity: onWindowFocusChanged");

        super.onWindowFocusChanged(hasFocus);
        // if (hasFocus) {
            mUiHelper.hide();
        // }
    }

    public void checkCloseLogo() {
        if (m_logo_view != null) {
            if (nativeIsInitComplete()) {
                runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mRootLayout.removeView(m_logo_view);
                            m_logo_view = null;
                            Log.d("drow", "init complete, remove logo view");
                        }
                    });
            }
        }
    }
    
    public boolean render_commit_begin() {
        Log.d("drow", "render_commit_begin");
        return mGLView.render_commit_begin();
    }

    public void render_commit_done() {
        Log.d("drow", "render_commit_done");
        mGLView.render_commit_done();
    }
    
    public void jumpToSetting() {
        Log.d("drow", "DrowActivity: jumpToSetting");

        Intent intent = new Intent();
        intent.setAction(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        try {
            startActivity(intent);  
        }
        catch(ActivityNotFoundException ex) {
            // The Android SDK doc says that the location settings activity
            // may not be found. In that case show the general settings.
            // General settings activity
            intent.setAction(Settings.ACTION_SETTINGS);
            try {
                startActivity(intent);
            }
            catch (Exception e) {
            }
        }
    }

    public void openWebView(String Url) {
        Log.d("drow", "DrowActivity: openWebView");

        Uri content_url = Uri.parse(Url); 
        Intent intent = new Intent(Intent.ACTION_VIEW, content_url);
        try {
            startActivity(intent);
        }
        catch (Exception e) {
        }
    }
    
    public boolean isSDCardAvailable() {
        Log.d("drow", "DrowActivity: isSDCardAvailable");

        boolean externalStorageAvailable = false;
        boolean externalStorageWriteable = false;

        String state = Environment.getExternalStorageState();

        if (Environment.MEDIA_MOUNTED.equals(state)) {
            // We can read and write the media
            externalStorageAvailable = externalStorageWriteable = true;
        } else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
            // We can only read the media
            externalStorageAvailable = true;
            externalStorageWriteable = false;
        } else {
            // Something else is wrong. It may be one of many other states, but all we need
            //  to know is we can neither read nor write
            externalStorageAvailable = externalStorageWriteable = false;
        }
        return externalStorageAvailable && externalStorageWriteable;
    }

    public void runOnGLThread(final Runnable runnable) {
        mGLView.queueEvent(runnable);
    }
            
    private static native void setJavaVM();
    private static native void setAndroidApk(String apk);
    private static native void setInternalDir(String path);
    private static native void setExternalDir(String path);
    private static native void setAssetManager(Object apk);
    private static native void nativeOnActivityResult(Object intent, int requestCode, int resultCode);
    private static native void nativeOnNewIntent(Object intent);

    public static native void nativeOnPause();
    public static native void nativeOnStop();
    public static native void nativeOnRestart();
    public static native void nativeOnStart();
    public static native void nativeInit(Activity activity);
    public static native boolean nativeIsInitComplete();
    public static native long nativeFixFrameTime();
    public static native boolean nativeIsStop(); 
    public static native boolean nativeRender();    
    public static native void nativeResize(int w, int h);
    public static native void nativeOnTouch(int action, int index, int x, int y);
    public static native void nativeOnKeyBack();
    public static native void nativeOnKeyMenu();
    public static native void nativeOnResume();
}

