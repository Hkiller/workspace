package com.drowgames.facebook_share;

public class FacebookShareListener {
    private long m_ptr;
    
    public FacebookShareListener(long ptr) {
        m_ptr = ptr;
    }

    public void destory() {
        m_ptr = 0;
    }
}

