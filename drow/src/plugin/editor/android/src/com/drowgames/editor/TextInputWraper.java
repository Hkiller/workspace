package com.drowgames.editor;

import android.app.Activity;
import android.util.Log;
import android.content.Context;
import android.content.ClipboardManager;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.TextWatcher;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.FrameLayout;
import android.widget.TextView.OnEditorActionListener;
import android.graphics.Canvas;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.view.View;
import android.view.KeyEvent;
import android.view.ViewGroup;
//import android.opengl.GLSurfaceView;
//import com.drowgames.helper.DrowGLSurfaceView;

public class TextInputWraper implements TextWatcher, OnEditorActionListener {
    private long m_ptr;
    private Activity mActivity;
    private EditText mTextField;
    
    TextInputWraper(long ptr, Activity activity) {
        m_ptr = ptr;
        mActivity = activity;
        
        mActivity.runOnUiThread(new Runnable() {
                public void run() {
                    mTextField = new EditText(mActivity) {
                            @Override
                            public boolean onKeyDown(final int pKeyCode, final KeyEvent pKeyEvent) {
                                super.onKeyDown(pKeyCode, pKeyEvent);

                                /* Let GlSurfaceView get focus if back key is input. */
                                if (pKeyCode == KeyEvent.KEYCODE_BACK) {
                                    Log.e("drow", "TextInputWraper: main view focuse 111");
                                }

                                Log.e("drow", "TextInputWraper: xxxx: key-code=" + pKeyCode);

                                return true;
                            }

                            // @Override  
                            // protected void onDraw(Canvas canvas) {
                            // }
                        };
                    
                    mTextField.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
                    mTextField.setCursorVisible(false);
                    mTextField.setOnEditorActionListener(TextInputWraper.this);
                    mTextField.setTranslationY(5000);

                    FrameLayout layout = ((FrameLayout)mActivity.getWindow().getDecorView());
                    layout.addView(mTextField);
                    
                    //mTextField.setVisibility(View.INVISIBLE);
                    //mActivity.requestFocus();
                    Log.e("drow", "TextInputWraper: crate text field success");
                }
            });
    }

    public void updateContext(final String text, final int passwd, final int number_only, final int maxlen) {
    	mActivity.runOnUiThread(new Runnable() {
            public void run() {
		        Log.e("drow", "TextInputWraper: showImeKeyBoard");
		
		        mTextField.removeTextChangedListener(TextInputWraper.this);
		
		        setTextFieldAttr(passwd, number_only);
		        if (maxlen > 0) {
		            mTextField.setFilters(new InputFilter[]{new InputFilter.LengthFilter(maxlen)});
		        }
		        else {
		            mTextField.setFilters(new InputFilter[]{});
		        }
		
		        mTextField.setText(text);
		        
		//         if(text != null && text.length() > 0) {
		//             if (text.charAt(text.length() - 1) == '\n') {
		//                 text = text.substring(0, text.length()-1);
		//             }
		//             mTextField.append(text);
		//         }
		
		        mTextField.addTextChangedListener(TextInputWraper.this);
		        
		        mTextField.requestFocus();
		        
		        InputMethodManager imm = (InputMethodManager)mActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
		        imm.showSoftInput(mTextField, 0);
            }
        });
    }
   
    public void updateSelection(final int begin_pos, final int end_pos) {
    	mActivity.runOnUiThread(new Runnable() {
            public void run() {
		        if (begin_pos < end_pos) {
		            Log.e("drow", "TextInputWraper: updateSelection: to range " + begin_pos + "~" + end_pos);
		            mTextField.setSelection(begin_pos, end_pos);
		        }
		        else {
		            if (end_pos < 0 || end_pos > mTextField.getText().length()) {
		                Log.e("drow", "TextInputWraper: updateSelection: to last " + mTextField.getText().length());
		                mTextField.setSelection(mTextField.getText().length());
		            }
		            else {
		                Log.e("drow", "TextInputWraper: updateSelection: to pos " + end_pos);
		                mTextField.setSelection(end_pos);
		            }
		        }
            }
        });
    }
   
    public void closeKeyBoard() {	
    	mActivity.runOnUiThread(new Runnable() {
            public void run() {
                Log.e("drow", "TextInputWraper: hideImeKeyBoard");

                mTextField.removeTextChangedListener(TextInputWraper.this);
                mTextField.setText("");
                
                InputMethodManager imm = (InputMethodManager)mActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mTextField.getWindowToken(), 0);
                //mActivity.requestFocus();     	
            }
        });
    }

    public void copyToClipboard(final String content) {
    	mActivity.runOnUiThread(new Runnable() {
            public void run() {
                ClipboardManager cmb = (ClipboardManager)mActivity.getSystemService(Context.CLIPBOARD_SERVICE);  
                cmb.setText(content);        	
            }
        });
    }
    
    public String getFromClipboard() {
        ClipboardManager cmb = (ClipboardManager)mActivity.getSystemService(Context.CLIPBOARD_SERVICE);  
        return cmb.getText().toString();     	
    }
    
	public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        Log.e("drow", "TextInputWraper: beforeTextChanged");
	}

	public void onTextChanged(CharSequence s, int start, int before, int count) {
        Log.e("drow", "TextInputWraper: onTextChanged");
	}

	public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
        Log.e("drow", "TextInputWraper: onEditorAction");
        
        /* If user didn't set keyboard type, this callback will be invoked twice with 'KeyEvent.ACTION_DOWN' and 'KeyEvent.ACTION_UP'. */
        if (actionId != EditorInfo.IME_NULL || (actionId == EditorInfo.IME_NULL && event != null && event.getAction() == KeyEvent.ACTION_DOWN)) {
            nativeCommitContent(m_ptr, mTextField.getText().toString());
            nativeCommitSelection(m_ptr, mTextField.getSelectionStart(), mTextField.getSelectionStart());
            //nativeClose(m_ptr);
            InputMethodManager imm = (InputMethodManager)mActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.hideSoftInputFromWindow(mTextField.getWindowToken(), 0);
            //closeKeyBoard();
            return true;
        }
        return false;
	}

	public void afterTextChanged(Editable s) {
        if (mTextField.getParent() == null) {
            Log.e("drow", "TextInputWraper: afterTextChanged: control is already hide");
            return;
        }
        
        String text = s.toString();
        Log.e("drow", "TextInputWraper: afterTextChanged");
        nativeCommitContent(m_ptr, text);
        nativeCommitSelection(m_ptr, mTextField.getSelectionStart(), mTextField.getSelectionEnd());
	}

    private void setTextFieldAttr(int passwd, int number_only) {
    	int nEtAttr = 0;

        if (number_only != 0) {
    		nEtAttr |= (InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_SIGNED | InputType.TYPE_NUMBER_FLAG_DECIMAL);
        }
        else if (passwd != 0) {
    		nEtAttr |= InputType.TYPE_TEXT_VARIATION_PASSWORD;
        }
    	else {
            nEtAttr |= InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_NORMAL;
        }
        
    	mTextField.setInputType(nEtAttr);
    }
    
    public static native void nativeCommitContent(long ptr, String text);
    public static native void nativeCommitSelection(long ptr, int begin_pos, int end_pos);
    public static native void nativeClose(long ptr);
}
