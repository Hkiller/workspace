#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_touch_track.h"
#include "RGUIToggle.h"
#include "RGUIWindow.h"
#include "RGUITabPage.h"
#include "RGUITab.h"

/*
constructor
*/
RGUITab::RGUITab() {
	mToggleDock = 			TD_TP;
	mToggleGrapUnit =  	RGUIUnitVec2::Zero;
	mToggleRelaUnit = 		RGUIUnitVec2::Zero;
	mToggleTemplateName = 	"";

    mToggleGrapReal = RVector2::Zero;
    mToggleRelaReal = RVector2::Zero;
	mToggleTemplate = NULL;
}

/*
page
*/
void	RGUITab::AddTabPage			( const ::std::string& text )
{
	RGUITabPage* tabPage = create<RGUITabPage>();
	AddChild(tabPage);
	tabPage->setName("RGUITabPage");
	tabPage->SetToggleText(text);
	tabPage->SetToggleTemplateName(mToggleTemplateName);

	//确保显示正确 
	tabPage->Hide();
	tabPage->Show();
}

/*
load & save & clone
*/
void RGUITab::Load( ui_data_control_t control ) {
    RGUIControl::Load(control);

    if (type() == ui_control_type_tab) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        // if (!mTemplateLink) {
            mToggleDock = data.data.tab.toggle_dock;
            mToggleGrapUnit = data.data.tab.toggle_grap_unit;
            mToggleRelaUnit = data.data.tab.toggle_rela_unit;
            mToggleTemplateName = ui_data_control_msg(control, data.data.tab.toggle_template_id);
            mToggleGrapReal = data.data.tab.toggle_grap_real;
            mToggleRelaReal = data.data.tab.toggle_rela_real;
        // }
    }
}

/*
virtual
*/
void RGUITab::PerformLayout(ui_vector_2_t client_sz)
{
    client_sz->x = client_sz->y = 0.0f;

	if (mToggleTemplate)
	{
        RGUIUnitVec2 toggleUnit = RGUIUnitVec2::ToUnit(
            ui_data_control_data(mToggleTemplate)->basic.accept_sz_ls,
            GetEditorRealSZ(),
            GetEditorRealSZ(),
            ui_data_control_data(mToggleTemplate)->basic.editor_sz);
        
		RVector2     toggleSize = toggleUnit.ToReal(
            ui_data_control_data(mToggleTemplate)->basic.accept_sz_ls,
            GetRenderRealSZ(),
            GetEditorRealSZ());
        
		int32_t  w  = (int32_t)toggleSize.w;
		int32_t  h  = (int32_t)toggleSize.h;
		ui_rect rt = GetRenderTextRT();
		switch ((uint8_t)mToggleDock)
		{
		case TD_TP:	rt.lt.y += h;	break;
		case TD_BM:	rt.rb.y -= h;	break;
		case TD_LT:	rt.lt.x += w;	break;
		case TD_RT:	rt.rb.x -= w;	break;
		}

		//布局 
		RVector2 pt = RVector2(rt.lt.x,	rt.lt.y);
        RVector2 sz = RVector2(ui_rect_width(&rt),	ui_rect_height(&rt));

        plugin_ui_control_it child_it;
        plugin_ui_control_childs(control(), &child_it);

        for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
            child;
            child = plugin_ui_control_it_next(&child_it))
        {
            RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);

			c->SetRenderRealPT(pt);
			c->SetRenderRealSZ(sz);
		}
	}
}

/*
call back
*/
void RGUITab::OnLoadProperty() {
	RGUIControl::OnLoadProperty();
    
	//得到模版指针 
	mToggleTemplate = GetTemplate(mToggleTemplateName.c_str());
}

void RGUITab::DoClick(ui_vector_2 const & click_pt) {
	if (mToggleTemplate == NULL)
		return;

    RVector2 curr = GetRenderRealSZ();
    RVector2 orig = GetEditorRealSZ();

    RGUIUnitVec2 toggleUnit =  RGUIUnitVec2::ToUnit(
        ui_data_control_data(mToggleTemplate)->basic.accept_sz_ls,
        orig,
        orig,
        ui_data_control_data(mToggleTemplate)->basic.editor_sz);
    
    RVector2     toggleSize =      toggleUnit.ToReal(
        ui_data_control_data(mToggleTemplate)->basic.accept_sz_ls, curr, orig);
	RVector2     toggleRela = mToggleRelaUnit.ToReal(WasAcceptPTLS(),                      curr, orig);
	RVector2     toggleGrap = mToggleGrapUnit.ToReal(WasAcceptPTLS(),                      curr, orig);

    ui_rect    pd = GetClientRealPD();
	ui_rect    rt = GetRenderRealRT();
	RVector2 pt = GetRenderRealPTAbs();

	ui_rect drawRT;
	switch ((uint8_t)mToggleDock)
	{
	case TD_TP: 
	case TD_LT:
		drawRT.lt.x = (int32_t)pt.x + pd.lt.x + (int32_t)toggleRela.x;
		drawRT.lt.y = (int32_t)pt.y + pd.lt.y + (int32_t)toggleRela.y;
		break;
	case TD_BM:
		drawRT.lt.x = (int32_t)pt.x + pd.lt.x + (int32_t)toggleRela.x;
		drawRT.lt.y = (int32_t)pt.y - pd.rb.y + (int32_t)toggleRela.y + ui_rect_height(&rt) - (int32_t)toggleSize.h;
		break;
	case TD_RT:
		drawRT.lt.x = (int32_t)pt.x - pd.rb.x + (int32_t)toggleRela.x + ui_rect_width(&rt) - (int32_t)toggleSize.w;
		drawRT.lt.y = (int32_t)pt.y + pd.lt.x + (int32_t)toggleRela.y;
		break;
	}

    plugin_ui_control_it child_it;
    plugin_ui_control_childs(control(), &child_it);

    for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
        child;
        child = plugin_ui_control_it_next(&child_it))
    {
        RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
		RGUITabPage* page = dynamic_cast<RGUITabPage*>(c);
		ui_data_control_t togg = GetTemplate(page->GetToggleTemplateName().c_str());
		if (togg == NULL)
			togg  = mToggleTemplate;

        RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
            ui_data_control_data(togg)->basic.accept_sz_ls,
            orig,
            orig,
            ui_data_control_data(togg)->basic.editor_sz);
		RVector2     size = unit.ToReal(ui_data_control_data(togg)->basic.accept_sz_ls, curr, orig);
		int32_t w = (int32_t)size.w;
		int32_t h = (int32_t)size.h;

        drawRT.rb.x = drawRT.lt.x + w;
        drawRT.rb.y = drawRT.lt.y + h;

		//切换页面 
		if (ui_rect_is_contain_pt(&drawRT, (ui_vector_2_t)&click_pt))
		{
			page->Show();
			break;
		}

		w += (int32_t)toggleGrap.w;
		h += (int32_t)toggleGrap.h;

		switch ((uint8_t)mToggleDock)
		{
		case TD_TP:
		case TD_BM:
                drawRT.lt.x += w;
                drawRT.rb.x += w;
			break;
		case TD_LT:
		case TD_RT:
                drawRT.lt.y += h;
                drawRT.rb.y += h;
			break;
		}
	}
}

/*
method
*/
void	RGUITab::RenderTail			( ui_runtime_render_t ctx, ui_rect_t rect )
{
	// RGUIControl::RenderTail(ctx, rect);
	// if (mToggleTemplate == NULL)
	// 	return;

    // RVector2 curr = GetRenderRealSZ();
    // RVector2 orig = GetEditorRealSZ();

    // RGUIUnitVec2 toggleUnit =  RGUIUnitVec2::ToUnit(
    //     ui_data_control_data(mToggleTemplate)->basic.accept_sz_ls,
    //     orig,
    //     orig,
    //     ui_data_control_data(mToggleTemplate)->basic.editor_sz);
    // RVector2     toggleSize = toggleUnit.ToReal(
    //         ui_data_control_data(mToggleTemplate)->basic.accept_sz_ls, curr, orig);
    // RVector2     toggleRela = mToggleRelaUnit.ToReal(WasAcceptPTLS(),                      curr, orig);
    // RVector2     toggleGrap = mToggleGrapUnit.ToReal(WasAcceptPTLS(),                      curr, orig);

	// //绘制标签 
    // RRect pd = GetClientRealPD();
	// RRect rt = GetRenderRealRT();

	// RRect drawRT;
	// switch ((uint8_t)mToggleDock)
	// {
	// case TD_TP:
	// case TD_LT:
	// 	drawRT.LT =   pd.LT + (int32_t)toggleRela.x;
	// 	drawRT.TP =   pd.TP + (int32_t)toggleRela.y;
	// 	break;
	// case TD_BM:
	// 	drawRT.LT =   pd.LT + (int32_t)toggleRela.x;
	// 	drawRT.TP = - pd.BM + (int32_t)toggleRela.y + rt.GetH() - (int32_t)toggleSize.h;
	// 	break;
	// case TD_RT:
	// 	drawRT.LT = - pd.RT + (int32_t)toggleRela.x + rt.GetW() - (int32_t)toggleSize.w;
	// 	drawRT.TP =   pd.TP + (int32_t)toggleRela.y;
	// 	break;
	// }

    // plugin_ui_control_it child_it;
    // plugin_ui_control_childs(control(), &child_it);

    // for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
    //     child;
    //     child = plugin_ui_control_it_next(&child_it))
    // {
    //     RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
	// 	RGUITabPage* page = dynamic_cast<RGUITabPage*>(c);
	// 	ui_data_control_t  togg = GetTemplate(page->GetToggleTemplateName().c_str());
	// 	if (togg == NULL)
	// 		togg  = mToggleTemplate;

    //     RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
    //         ui_data_control_data(togg)->basic.accept_sz_ls,
    //         orig,
    //         orig,
    //         ui_data_control_data(togg)->basic.editor_sz);
    //     RVector2     size = unit.ToReal(
    //         ui_data_control_data(togg)->basic.accept_sz_ls, curr, orig);
	// 	int32_t w = (int32_t)size.w;
	// 	int32_t h = (int32_t)size.h;

	// 	drawRT.SetW(w);
	// 	drawRT.SetH(h);

	// 	//绘制 
    //     if (c->WasVisible())
    //     {
    //         TabToggleRender(
    //             ctx, rect,
    //             page, 
    //             drawRT);
    //     }

	// 	w += (int32_t)toggleGrap.w;
	// 	h += (int32_t)toggleGrap.h;

	// 	switch ((uint8_t)mToggleDock)
	// 	{
	// 	case TD_TP:
	// 	case TD_BM:
	// 		drawRT.OftX(w);
	// 		break;
	// 	case TD_LT:
	// 	case TD_RT:
	// 		drawRT.OftY(h);
	// 		break;
	// 	}
	// }

    // plugin_ui_control_childs(control(), &child_it);

    // for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
    //     child;
    //     child = plugin_ui_control_it_next(&child_it))
    // {
    //     RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);

    //     RGUITabPage* page = dynamic_cast<RGUITabPage*>(c);
    //     ui_data_control_t  togg = GetTemplate(page->GetToggleTemplateName().c_str());
    //     if (togg == NULL)
    //         togg  = mToggleTemplate;

    //     RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
    //         ui_data_control_data(togg)->basic.accept_sz_ls,
    //         orig,
    //         orig,
    //         ui_data_control_data(togg)->basic.editor_sz);
    //     RVector2     size = unit.ToReal(
    //         ui_data_control_data(togg)->basic.accept_sz_ls, curr, orig);
    //     int32_t w = (int32_t)size.w;
    //     int32_t h = (int32_t)size.h;

    //     drawRT.SetW(w);
    //     drawRT.SetH(h);

    //     //绘制 
    //     if (c->WasVisible() == false)
    //     {
    //         TabToggleRender(
    //             ctx, rect,
    //             page, 
    //             drawRT);
    //     }

    //     w += (int32_t)toggleGrap.w;
    //     h += (int32_t)toggleGrap.h;

    //     switch ((uint8_t)mToggleDock)
    //     {
    //     case TD_TP:
    //     case TD_BM:
    //         drawRT.OftX(w);
    //         break;
    //     case TD_LT:
    //     case TD_RT:
    //         drawRT.OftY(h);
    //         break;
    //     }
    // }
}

void	RGUITab::TabToggleRender		(ui_runtime_render_t ctx, ui_rect_t rect,  RGUITabPage* page, const ui_rect& rt )
{
	//确定模版 
	ui_data_control_t toggle = GetTemplate(page->GetToggleTemplateName().c_str());
	if (toggle == NULL)
		toggle  = mToggleTemplate;

	//背景
    //TODO: (Loki)
	// RGUIFrameRef frame   = page->WasVisible() 
	// 	? toggle->GetDownFrame() 
	// 	: toggle->GetBackFrame();
	// DrawFrame(ctx, rect, rt, RColor::White, frame);

    // RVector2 curr = RVector2((float)rt.GetW(), (float)rt.GetH());
    // RVector2 orig = ui_data_control_data(toggle)->basic.editor_sz;

	// //文本 
	// RFontDraw drawInfo   = page->WasVisible() 
	// 	? toggle->GetTextDownDraw() 
	// 	: toggle->GetTextBackDraw();

    // RVector2  fontGrap   = page->WasVisible()
    //     ? toggle->GetTextDownGrap().ToReal(false, curr, orig)
    //     : toggle->GetTextBackGrap().ToReal(false, curr, orig);

	// RFontInfo fontInfo   = toggle->GetTextFontInfo();
	// fontInfo.size		  = (int32_t)(RGUIUnitVec2(RGUIUnit::Zero, toggle->GetTextFontUnit()).ToReal(false, curr, orig).h);
	// fontInfo.outline	  = ((drawInfo.drawFlag & TR_STROKE) == 0) ? 0 : drawInfo.strokeWidth;

	// RFontDraw fontDraw   = drawInfo;
    // fontDraw.horzGrap     = (int32_t)fontGrap.x;
    // fontDraw.vertGrap     = (int32_t)fontGrap.y;
	// fontDraw.normalColor *= GetRenderRealColor();
	// fontDraw.shadowColor *= GetRenderRealColor();
	// fontDraw.strokeColor *= GetRenderRealColor();

	// RRect calcRT = rt;
	// //RRect textPD = toggle->GetClientRealPD();
	// //calcRT.LT += textPD.LT;
	// //calcRT.TP += textPD.TP;
	// //calcRT.RT -= textPD.RT;
	// //calcRT.BM -= textPD.BM;

	// //计算矩阵 
	// RVector2	t = GetRenderRealPTAbs();
	// RVector2   s = GetRenderRealScale();
	// RMatrix4x4 m;
	// m.SetIdentity();
	// m.m11 = s.x;
	// m.m22 = s.y;
	// m.m14 = t.x;
	// m.m24 = t.y;

	// RRect realRT = 
	// RFontImpl::GetIns()->MearsureRect(
	// 	page->GetToggleText(), 
	// 	fontInfo, 
	// 	fontDraw, 
	// 	calcRT, 
	// 	true, 
	// 	ui_data_control_data(toggle)->GetTextAlign());

	// RFontImpl::GetIns()->DrawTextTrans(
    //     ctx, rect,
	// 	page->GetToggleText(),
	// 	fontInfo,
	// 	fontDraw,
	// 	realRT,
	// 	true,
	// 	m);
}

void RGUITab::on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz) {
    RGUITab * tab = (RGUITab*)plugin_ui_control_product(control);
    tab->PerformLayout(client_sz);
}

void RGUITab::on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIControl::on_mouse_down(ctx, from_control, event);

    if(ctx == from_control) {
        RGUITab * c = cast<RGUITab>(from_control);
        plugin_ui_touch_track_t track = plugin_ui_control_touch_track(from_control);
        assert(track);
        c->DoClick(*plugin_ui_touch_track_cur_pt(track));
    }
}

void RGUITab::setup(plugin_ui_control_meta_t meta) {
    plugin_ui_control_meta_set_layout(meta, on_layout);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_down, plugin_ui_event_scope_self, on_mouse_down);
}
