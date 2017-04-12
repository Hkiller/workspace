package com.drowgames.helper;

import java.util.Vector;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.EGL14;
import android.os.Handler;
import android.os.Message;
import android.support.v4.view.MotionEventCompat;
import android.app.Activity;
import android.util.AttributeSet;
import android.util.Log;
import android.graphics.Point;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.opengl.GLSurfaceView.EGLContextFactory;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class DrowGLSurfaceView extends GLSurfaceView {
    /*consts*/
    private final static long NANOSECONDSPERSECOND = 1000000000L;
    private final static long NANOSECONDSPERMICROSECOND = 1000000;

    /*env*/
    private DrowActivity mActivity;
    
    /*tick*/
    private long mLastTickInNanoSeconds;

    /*egl attributes */
    // public EGL10 mEgl;
    // public EGLDisplay mEglDisplay;
    // public EGLContext mWorkContext;
    // public EGLSurface mNextSurface;
    
    public DrowGLSurfaceView(Context context) {
        super(context);
        mActivity = (DrowActivity)context;
        initView(context);
    }

    protected void initView(Context context) {
        setEGLContextClientVersion(2); // Pick an OpenGL ES 2.0 context.
		setFocusableInTouchMode(true);
        
        setEGLContextFactory(new EGLContextFactory() {
                public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig config) {
                    printConfig(egl, display, config);

                    // mEgl = egl;
                    // mEglDisplay = display;
                    
                    int[] attrib_list = {0x3098 /*EGL_CONTEXT_CLIENT_VERSION*/, 2, EGL10.EGL_NONE };
                    EGLContext mainContext = egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT, attrib_list);
                    // mWorkContext =  egl.eglCreateContext(display, config, mMainContext, attrib_list);

                    // Point displaySize = mActivity.displaySize();
                    
                    // int pbufferAttribs[] = {
                    //     EGL10.EGL_WIDTH, displaySize.x,
                    //     EGL10.EGL_HEIGHT, displaySize.y,
                    //     EGL14.EGL_TEXTURE_TARGET, EGL14.EGL_NO_TEXTURE,
                    //     EGL14.EGL_TEXTURE_FORMAT, EGL14.EGL_NO_TEXTURE,
                    //     EGL10.EGL_NONE
                    // };
                    // mNextSurface = egl.eglCreatePbufferSurface(display, config, pbufferAttribs);
                    
                    return mainContext;
                }

                public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
                    egl.eglDestroyContext(display, context);
                    
                    // egl.eglDestroyContext(display, mWorkContext);
                    // egl.eglDestroySurface(display, mWorkSurface);
                    // mEgl = null;
                    // mEglDisplay = null;
                    // mWorkContext = null;
                }
            });
        
        setRenderer(new Renderer() {
                public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                    Log.d("drow", "onSurfaceCreated");
                    DrowActivity.nativeInit(mActivity);

                    Point displaySize = mActivity.displaySize();
                    DrowActivity.nativeResize(displaySize.x, displaySize.y);
                    
                    DrowActivity.nativeOnResume();
                    
                    mLastTickInNanoSeconds = System.nanoTime();
                    mActivity.mInit = true;
                }

                public void onSurfaceChanged(GL10 gl, int w, int h) {
                    Log.d("drow", "onSurfaceChanged: screen-size=(" + w  + "," + h + ")");
                    DrowActivity.nativeResize(w, h);
                }

                public void onDrawFrame(GL10 gl) {
                    /*检查是否程序是否停止 */
                    if (DrowActivity.nativeIsStop()) {
                        mActivity.finish();
                        return;
                    }

                    mActivity.checkCloseLogo();
                    
                    final long frameTime = DrowActivity.nativeFixFrameTime() * NANOSECONDSPERMICROSECOND;
                    final long interval = System.nanoTime() - mLastTickInNanoSeconds;
                        
                    if (interval < frameTime) {
                        try {
                            Thread.sleep((frameTime - interval) / NANOSECONDSPERMICROSECOND);
                        } catch (final Exception e) {
                        }
                    }
                    
                    /*
                     * Render time MUST be counted in, or the FPS will slower than appointed.
                     */
                    mLastTickInNanoSeconds = System.nanoTime();
                    DrowActivity.nativeRender();
                }
            });
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // for touch event
    ///////////////////////////////////////////////////////////////////////////
    @Override
    public  boolean onTouchEvent(final MotionEvent event)  {
        if (!mActivity.mInit) return false;
        final int action = MotionEventCompat.getActionMasked(event);
        final int pointerNumber = event.getPointerCount();  
        final Vector<Integer> list_x = new Vector(); 
        final Vector<Integer> list_y = new Vector(); 
        final Vector<Integer> list_pointerId = new Vector(); ;
        for (int i = 0; i < pointerNumber; i++) {
        	list_x.addElement((int)event.getX(i));
        	list_y.addElement((int)event.getY(i));
        	list_pointerId.addElement((int)event.getPointerId(i));
        }
        
        queueEvent(new Runnable() {
                @Override
                public void run() {
                    switch (action) {
                    case MotionEvent.ACTION_POINTER_DOWN:
                        for (int i = 0; i < list_pointerId.size(); i++) {
                            DrowActivity.nativeOnTouch(0, list_pointerId.elementAt(i), list_x.elementAt(i), list_y.elementAt(i));
                        }
                        break;
                    case MotionEvent.ACTION_POINTER_UP:
                        for (int i = 0; i < list_pointerId.size(); i++) {
                            DrowActivity.nativeOnTouch(2, list_pointerId.elementAt(i), list_x.elementAt(i), list_y.elementAt(i));
                        }
                        break;
                    case MotionEvent.ACTION_DOWN:
                        for (int i = 0; i < list_pointerId.size(); i++) {
                            DrowActivity.nativeOnTouch(0, list_pointerId.elementAt(i), list_x.elementAt(i), list_y.elementAt(i));
                        }
                        break;
                    case MotionEvent.ACTION_MOVE:
                        for (int i = 0; i < list_pointerId.size(); i++) {
                            DrowActivity.nativeOnTouch(1, list_pointerId.elementAt(i), list_x.elementAt(i), list_y.elementAt(i));
                        }
                        break;
                    case MotionEvent.ACTION_UP:
                    case MotionEvent.ACTION_CANCEL:
                        for (int i = 0; i < list_pointerId.size(); i++) {
                            DrowActivity.nativeOnTouch(2, list_pointerId.elementAt(i), list_x.elementAt(i), list_y.elementAt(i));
                        }
                    	break;
                    }
                }
            });
            
		return true;
	}

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (!mActivity.mInit) return false;
        
    	if(keyCode == KeyEvent.KEYCODE_BACK) {
            DrowActivity.nativeOnKeyBack();
            return true;
        }
        else if (keyCode == KeyEvent.KEYCODE_MENU) {
            DrowActivity.nativeOnKeyMenu();
        	return true;
    	}
    	else {
            return super.onKeyDown(keyCode, event);
        }
    }  

    public boolean render_commit_begin() {
        // if (mEgl == null) return false;

        // if (!mEgl.eglMakeCurrent(mEglDisplay, mNextSurface, mNextSurface, mWorkContext)) {
        //     Log.e("drow", "render_commit_begin: eglMakeCurrent fail!");
        //     return false;
        // }
        return true;
    }

    public void render_commit_done() {
        // if (mEgl == null) return;
        
        // mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
    }
    
    private void printConfig(EGL10 egl, EGLDisplay display, EGLConfig config) {
        Log.d("drow", "DrowRenderer: dump config");
        int[] attributes = {
            EGL10.EGL_BUFFER_SIZE,
            EGL10.EGL_ALPHA_SIZE,
            EGL10.EGL_BLUE_SIZE,
            EGL10.EGL_GREEN_SIZE,
            EGL10.EGL_RED_SIZE,
            EGL10.EGL_DEPTH_SIZE,
            EGL10.EGL_STENCIL_SIZE,
            EGL10.EGL_CONFIG_CAVEAT,
            EGL10.EGL_CONFIG_ID,
            EGL10.EGL_LEVEL,
            EGL10.EGL_MAX_PBUFFER_HEIGHT,
            EGL10.EGL_MAX_PBUFFER_PIXELS,
            EGL10.EGL_MAX_PBUFFER_WIDTH,
            EGL10.EGL_NATIVE_RENDERABLE,
            EGL10.EGL_NATIVE_VISUAL_ID,
            EGL10.EGL_NATIVE_VISUAL_TYPE,
            0x3030, // EGL10.EGL_PRESERVED_RESOURCES,
            EGL10.EGL_SAMPLES,
            EGL10.EGL_SAMPLE_BUFFERS,
            EGL10.EGL_SURFACE_TYPE,
            EGL10.EGL_TRANSPARENT_TYPE,
            EGL10.EGL_TRANSPARENT_RED_VALUE,
            EGL10.EGL_TRANSPARENT_GREEN_VALUE,
            EGL10.EGL_TRANSPARENT_BLUE_VALUE,
            0x3039, // EGL10.EGL_BIND_TO_TEXTURE_RGB,
            0x303A, // EGL10.EGL_BIND_TO_TEXTURE_RGBA,
            0x303B, // EGL10.EGL_MIN_SWAP_INTERVAL,
            0x303C, // EGL10.EGL_MAX_SWAP_INTERVAL,
            EGL10.EGL_LUMINANCE_SIZE,
            EGL10.EGL_ALPHA_MASK_SIZE,
            EGL10.EGL_COLOR_BUFFER_TYPE,
            EGL10.EGL_RENDERABLE_TYPE,
            0x3042 // EGL10.EGL_CONFORMANT
        };
        String[] names = {
            "EGL_BUFFER_SIZE",
            "EGL_ALPHA_SIZE",
            "EGL_BLUE_SIZE",
            "EGL_GREEN_SIZE",
            "EGL_RED_SIZE",
            "EGL_DEPTH_SIZE",
            "EGL_STENCIL_SIZE",
            "EGL_CONFIG_CAVEAT",
            "EGL_CONFIG_ID",
            "EGL_LEVEL",
            "EGL_MAX_PBUFFER_HEIGHT",
            "EGL_MAX_PBUFFER_PIXELS",
            "EGL_MAX_PBUFFER_WIDTH",
            "EGL_NATIVE_RENDERABLE",
            "EGL_NATIVE_VISUAL_ID",
            "EGL_NATIVE_VISUAL_TYPE",
            "EGL_PRESERVED_RESOURCES",
            "EGL_SAMPLES",
            "EGL_SAMPLE_BUFFERS",
            "EGL_SURFACE_TYPE",
            "EGL_TRANSPARENT_TYPE",
            "EGL_TRANSPARENT_RED_VALUE",
            "EGL_TRANSPARENT_GREEN_VALUE",
            "EGL_TRANSPARENT_BLUE_VALUE",
            "EGL_BIND_TO_TEXTURE_RGB",
            "EGL_BIND_TO_TEXTURE_RGBA",
            "EGL_MIN_SWAP_INTERVAL",
            "EGL_MAX_SWAP_INTERVAL",
            "EGL_LUMINANCE_SIZE",
            "EGL_ALPHA_MASK_SIZE",
            "EGL_COLOR_BUFFER_TYPE",
            "EGL_RENDERABLE_TYPE",
            "EGL_CONFORMANT"
        };
        int[] value = new int[1];
        for (int i = 0; i < attributes.length; i++) {
            int attribute = attributes[i];
            String name = names[i];
            if (egl.eglGetConfigAttrib(display, config, attribute, value)) {
                Log.d("drow", String.format("  %s: %d\n", name, value[0]));
            }
            else {
                Log.d("drow", String.format("  %s: failed\n", name));
                while (egl.eglGetError() != EGL10.EGL_SUCCESS);
            }
        }
    }
}
