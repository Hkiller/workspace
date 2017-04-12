/* 
parent 
*/
inline
bool RGUIControl::WasMyAncestor( RGUIControl* control ) const {
    plugin_ui_control_t p = plugin_ui_control_parent(this->control());
    plugin_ui_control_t c = plugin_ui_control_parent(control->control());

    while(p) {
        if (p == c) return true;
        p = plugin_ui_control_parent(p);
    }

    return false;
}

inline
bool RGUIControl::WasDescendant(RGUIControl* control) const {
    plugin_ui_control_t p = plugin_ui_control_parent(control->control());
    plugin_ui_control_t c = this->control();

    while(p) {
        if (p == c) return true;
        p = plugin_ui_control_parent(p);
    }

    return false;
}

inline
RVector2 RGUIControl::GetRenderRealPTRel( const RVector2& absolute ) const {
	return absolute - GetRenderRealPTAbs();
}

/*
rectangle
*/
inline
ui_rect RGUIControl::GetClientTextRT(void) const {
    RVector2 sz = GetClientRealSZ();
	ui_rect rt = UI_RECT_INITLIZER(0, 0, sz.x, sz.y);
    ui_rect pd = GetClientRealPD();
	ui_rect r = UI_RECT_INITLIZER(rt.lt.x + pd.lt.x, rt.lt.y + pd.lt.y, rt.rb.x - pd.rb.x, rt.rb.y - pd.rb.y);
    return r;
}

inline
ui_rect RGUIControl::GetRenderTextRT(void) const {
	ui_rect rt = GetRenderRealRT();
    ui_rect pd = GetClientRealPD();
	ui_rect r = UI_RECT_INITLIZER(rt.lt.x + pd.lt.x, rt.lt.y + pd.lt.y, rt.rb.x - pd.rb.x, rt.rb.y - pd.rb.y);
    return r;
}

inline
ui_rect	RGUIControl::GetRenderTextRTAbs		( void ) const
{
	RVector2 s  = GetRenderRealScale();
	ui_rect    rt = GetRenderTextRT();

	//缩放
	rt.lt.x = (rt.lt.x * s.x);
	rt.lt.y = (rt.lt.y * s.y);
	rt.rb.x = (rt.rb.x * s.x);
	rt.rb.y = (rt.rb.y * s.y);

    ui_vector_2 pt = GetRenderRealPTAbs();
	rt.lt.x += pt.x;
	rt.lt.y += pt.y;
	rt.rb.x += pt.x;
	rt.rb.y += pt.y;
    return rt;
}
