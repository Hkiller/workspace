#ifndef DROW_UI_CONTROL_H
#define DROW_UI_CONTROL_H
#include "cpe/pal/pal_string.h"
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "cpepp/dr/CTypeUtils.hpp"
#include "render/utils/ui_color.h"
#include "render/utils/ui_vector_3.h"
#include "render/model/ui_data_layout.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin/ui/plugin_ui_module.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin/ui/plugin_ui_page.h"
#include "gdpp/app/Application.hpp"
#include "RGUI.h"
#include "RGUIUnitVec2.h"
#include "RGUIAnimation.hpp"
#include "RGUIControlActionBuilder.hpp"

template<typename T>
struct RGUIControlTraits;

class RGUIControl : public Cpe::Utils::Noncopyable {
	friend class RGUIWindow;
    friend class RGUIControlRepo;

public:
	RGUIControl(void);

    uint8_t type(void) const { return plugin_ui_control_type(control()); }

    plugin_ui_control_t control(void) const { return plugin_ui_control_from_product((void*)this); }
    plugin_ui_page_t page(void) const { return plugin_ui_control_page(control()); }
    plugin_ui_env_t ui_env(void) const { return plugin_ui_control_env(control()); }

    void updateCache(void) {
        plugin_ui_control_update_cache(control(), 0);
    }
    
    void checkUpdatCache(void) {
        plugin_ui_control_check_update_from_root(control());
    }

    Gd::App::Application & app(void) { return *(Gd::App::Application *)plugin_ui_control_app(control()); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application *)plugin_ui_control_app(control()); }

    ui_runtime_module_t runtime(void) const { return plugin_ui_control_runtime(control()); }
    ui_cache_manager_t cacheMgr(void) const { return plugin_ui_control_cache_mgr(control()); }
    ui_data_mgr_t dataMgr(void) const { return plugin_ui_control_data_mgr(control()); }

    void triggerNavigation(dr_data_t data = NULL) { plugin_ui_control_trigger_navigation(control(), data); }
    template<typename T>
    void triggerNavigation(T const & data) {
        dr_data d;
        d.m_meta = Cpe::Dr::MetaTraits<T>::META;
        d.m_data = (void *)&data;
        d.m_size = Cpe::Dr::MetaTraits<T>::data_size(data);
        triggerNavigation(&d);
    }

    static RGUIControl * cast(plugin_ui_control_t c) { return c ? (RGUIControl*)plugin_ui_control_product(c) : NULL; }
    static RGUIControl * clone(RGUIControl * other);

    static RGUIControl * create(plugin_ui_page_t page, uint8_t type);
    static RGUIControl * create(plugin_ui_page_t page, const char * src_path);
    static RGUIControl * create(plugin_ui_page_t page, ui_data_control_t data_control);

    void destory(void) { plugin_ui_control_free(control()); }

    RGUIControl * create(uint8_t type) { return create(page(), type); }
    RGUIControl * create(const char * src_path) { return create(page(), src_path); }
    RGUIControl * create(ui_data_control_t data_control) { return create(page(), data_control); }

    RGUIControl * create(uint32_t src_path_id);
    
    RGUICONTROL_DEF_BIND_FUN(click, mouse_click);
    RGUICONTROL_DEF_BIND_FUN(double_click, mouse_double_click);
    RGUICONTROL_DEF_BIND_FUN(long_push, mouse_long_push);
    RGUICONTROL_DEF_BIND_FUN(rise, mouse_up);
    RGUICONTROL_DEF_BIND_FUN(down, mouse_down);
	RGUICONTROL_DEF_BIND_FUN(drag, mouse_move);
    RGUICONTROL_DEF_BIND_FUN(move_begin, move_begin);
    RGUICONTROL_DEF_BIND_FUN(move_moving, move_moving);
    RGUICONTROL_DEF_BIND_FUN(move_done, move_done);
    RGUICONTROL_DEF_BIND_FUN(switch_changed, switch_changed);
    RGUICONTROL_DEF_BIND_FUN(swiper_changed, swiper_changed);
    RGUICONTROL_DEF_BIND_FUN(slider_changed, slider_changed);
    RGUICONTROL_DEF_BIND_FUN(toggle_click, toggle_click);    
    RGUICONTROL_DEF_BIND_FUN(radiobox_changed, radiobox_changed);
    RGUICONTROL_DEF_BIND_FUN(checkbox_changed, checkbox_changed);
    RGUICONTROL_DEF_BIND_FUN(editorbox_changed, editorbox_changed);
    RGUICONTROL_DEF_BIND_FUN(editorbox_enter, editorbox_enter);
    RGUICONTROL_DEF_BIND_FUN(vscroll_changed, vscroll_changed);
    RGUICONTROL_DEF_BIND_FUN(hscroll_changed, hscroll_changed);
    RGUICONTROL_DEF_BIND_FUN(progress_changed, progress_changed);
    RGUICONTROL_DEF_BIND_FUN(progress_done, progress_done);
    RGUICONTROL_DEF_BIND_FUN(list_item_show, list_item_show);
    RGUICONTROL_DEF_BIND_FUN(list_head_show, list_head_show);
    RGUICONTROL_DEF_BIND_FUN(list_tail_show, list_tail_show);
    RGUICONTROL_DEF_BIND_FUN(list_selection_changed, list_selection_changed);

    template<typename T>
    static T * create(plugin_ui_page_t page) {
        return dynamic_cast<T*>(create(page, (uint8_t)RGUIControlTraits<T>::TYPE_ID));
    }
    
    template<typename T>
    T * create(ui_data_control_t data_control) {
        return dynamic_cast<T*>(create(data_control));
    }

    template<typename T>
    T * create(const char * src_path) {
        return dynamic_cast<T*>(create(src_path));
    }

    template<typename T>
    T * create(uint32_t src_path_id) {
        return dynamic_cast<T*>(create(src_path_id));
    }
    
    template<typename T>
    T * create(void) const {
        return create<T>(page());
    }

    template<typename T>
    static T * cast(plugin_ui_control_t control) {
        return control ? dynamic_cast<T*>((RGUIControl *)plugin_ui_control_product(control)) : NULL;
    }

	/*
	template
	*/
    ui_data_control_t GetTemplate(const char * name);
    ui_data_control_t GetTemplate(uint32_t msg_id);
	ui_data_control_t GetTemplateLinkCtrl(void) { return plugin_ui_control_template(control()); }
	void SetTemplateLinkCtrl(ui_data_control_t ctrl) { plugin_ui_control_set_template(control(), ctrl); }

    /*basic*/
	const char * name(void) const { return plugin_ui_control_name(control()); }
	void setName(const char * name) { plugin_ui_control_set_name(control(), name); }


	/* tree */
	RGUIControl* GetParent(void) const { return cast(plugin_ui_control_parent(control())); }

	bool WasParent(RGUIControl* control) const { return GetParent() == control; }
	bool WasChild(RGUIControl* control) const { return control->GetParent() == this; }
	bool WasMyAncestor(RGUIControl* control) const;
	bool WasDescendant(RGUIControl* control) const;

	uint32_t GetChildCount(void) const { return (uint32_t)plugin_ui_control_child_count(control()); }
	RGUIControl* GetChildByName(const char * name) const { return cast(plugin_ui_control_find_child_by_name(control(), name)); }
	RGUIControl* RecChildByName(const char * name) const { return cast(plugin_ui_control_find_child_by_name_r(control(), name)); }
    RGUIControl * GetChildByPath(const char * page) const { return cast(plugin_ui_control_find_child_by_path(control(), page)); }
    
	void AddChild(RGUIControl* control, bool tail = true);
	void DelChild(RGUIControl* control, bool destroy = true);
	void DelChildren(bool destroy = true);

    /*pos*/
    RVector2 GetRenderRealPos(uint8_t policy) const { return plugin_ui_control_calc_local_pt_by_policy(control(), policy); }
        
	/* behavior */
	bool WasEnable(void) const { return plugin_ui_control_enable(control()) ? true : false; }
	void SetEnable(bool flag) { plugin_ui_control_set_enable(control(), flag ? 1 : 0); }

	bool WasVisible(void) const { return plugin_ui_control_visible(control()) ? true : false; }
	void SetVisible(bool flag) { plugin_ui_control_set_visible(control(), flag ? 1 : 0); }
	void Show(bool fireAnim = true);
	void Hide(bool fireAnim = true);

    bool WasAcceptGlobalResize(void) const { return plugin_ui_control_accept_global_resize(control()) ? true : false; }
	void SetAcceptGlobalResize(bool flag) { plugin_ui_control_set_accept_global_resize(control(), flag ? 1 : 0); }
	bool WasAcceptPTLS(void) const { return plugin_ui_control_accept_ptls(control()) ? true : false; }
	void SetAcceptPTLS(bool flag) { plugin_ui_control_set_accept_ptls(control(), flag ? 1 : 0); }
	bool WasAcceptSZLS(void) const { return plugin_ui_control_accept_szls(control()) ? true : false; }
	void SetAcceptSZLS(bool flag) { plugin_ui_control_set_accept_szls(control(), flag ? 1 : 0); }
	bool WasAcceptClip(void) const { return plugin_ui_control_accept_clip(control()) ? true : false; }
	void SetAcceptClip(bool flag) { plugin_ui_control_set_accept_clip(control(), flag ? 1 : 0); }
	bool WasParentClip(void) const { return plugin_ui_control_parent_clip(control()) ? true : false; }
	void SetParentClip(bool flag) { plugin_ui_control_set_parent_clip(control(), flag ? 1 : 0); }
	bool WasAcceptMove(void) const { return plugin_ui_control_accept_move(control()) ? true : false; }
	void SetAcceptMove(bool flag) { plugin_ui_control_set_accept_move(control(), flag ? 1 : 0); }
    bool WasAcceptClick(void) const { return plugin_ui_control_accept_click(control()) ? true : false; }
	void SetAcceptClick(bool flag) { plugin_ui_control_set_accept_click(control(), flag ? 1 : 0); }
    
	/* focus */
	bool HasFocus(void) const { return plugin_ui_control_has_focus(control()) ? true : false; }
	void ReqFocus(void) { plugin_ui_control_set_focus(control()); }

	/* catch */
	bool HasCatch(void) const { return plugin_ui_control_has_catch(control()) ? true : false; }
	bool HasCatchOrChildCatch(void) const { return plugin_ui_control_has_catch_r(control()) ? true : false; }

	/* pivot */
	RVector2 GetPivot(void) const { return *plugin_ui_control_pivot(control()); }
	void SetPivot(const RVector2& pivot) { plugin_ui_control_set_pivot(control(), (ui_vector_2_t)&pivot); }

	/* scale */
	RVector2 GetScale(void) const { return *plugin_ui_control_scale(control()); }
	void SetScale(const RVector2& scale) { plugin_ui_control_set_scale(control(), (ui_vector_2_t)&scale); }

	/* angle */
    ui_vector_3 GetAngle(void) const { return *plugin_ui_control_angle(control()); }
	void SetAngle( const ui_vector_3& angle )  { plugin_ui_control_set_angle(control(), (ui_vector_3_t)&angle); }

    /*all frame*/
    struct ui_vector_2 const & AllFramePos(void) const { return * plugin_ui_control_all_frame_pos(control()); }
    void SetAllFramePos(struct ui_vector_2 const & pos) { plugin_ui_control_set_all_frame_pos(control(), (ui_vector_2_t)&pos); }

    struct ui_vector_2 const & AllFrameScale(void) const { return * plugin_ui_control_all_frame_pos(control()); }
    void SetAllFrameScale(struct ui_vector_2 const & scale) { plugin_ui_control_set_all_frame_pos(control(), (ui_vector_2_t)&scale); }
    
	/*
	frame
	*/
	plugin_ui_control_frame_t findFrame(const char * name) { return plugin_ui_control_frame_find_by_name(control(), name); }
	plugin_ui_control_frame_t findFrame(ui_vector_2 const & pt) { return plugin_ui_control_frame_find_by_local_pt(control(), (ui_vector_2_t)&pt); }    
    
	plugin_ui_control_frame_t SetBackFrame(
        ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL) {
        return SetFrame(render_obj_ref, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, pos_policy, aspect);
    }
	plugin_ui_control_frame_t SetBackFrame(const char * res, plugin_ui_aspect_t aspect = NULL) {
        return SetFrame(res, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, aspect);
    }

	plugin_ui_control_frame_t SetFloatFrame(
		ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL) {
			return SetFrame(render_obj_ref, plugin_ui_control_frame_layer_float, plugin_ui_control_frame_usage_normal, pos_policy, aspect);
	}
	plugin_ui_control_frame_t SetFloatFrame(const char * res, plugin_ui_aspect_t aspect = NULL) {
		return SetFrame(res, plugin_ui_control_frame_layer_float, plugin_ui_control_frame_usage_normal, aspect);
	}

	plugin_ui_control_frame_t AddBackFrame(
        ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL) {
        return AddFrame(render_obj_ref, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, pos_policy, aspect);
    }
	plugin_ui_control_frame_t AddBackFrame(const char * res, plugin_ui_aspect_t aspect = NULL) {
        return AddFrame(res, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, aspect);
    }
	void ClearBackFrame(plugin_ui_aspect_t aspect = NULL) {
        ClearFrame(plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, aspect);
    }
        
    /**/
	plugin_ui_control_frame_t SetForeFrame(
        ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL) {
        return SetFrame(render_obj_ref, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, pos_policy, aspect);
    }
	plugin_ui_control_frame_t SetForeFrame(const char * res, plugin_ui_aspect_t aspect = NULL) {
        return SetFrame(res, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, aspect);
    }
	plugin_ui_control_frame_t AddForeFrame(
        ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL) {
        return AddFrame(render_obj_ref, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, pos_policy, aspect);
    }
	plugin_ui_control_frame_t AddForeFrame(const char * res, plugin_ui_aspect_t aspect = NULL) {
        return AddFrame(res, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, aspect);
    }
	void ClearForeFrame(plugin_ui_aspect_t aspect = NULL) {
        ClearFrame(plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, aspect);
    }

	plugin_ui_control_frame_t SetFrame(
        ui_runtime_render_obj_ref_t render_obj_ref, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, 
        uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL);
	plugin_ui_control_frame_t SetFrame(
        const char * res, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, 
        plugin_ui_aspect_t aspect = NULL);
	plugin_ui_control_frame_t AddFrame(
        ui_runtime_render_obj_ref_t render_obj_ref, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, 
        uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL);
	plugin_ui_control_frame_t AddFrame(
        const char * res, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect = NULL);
	plugin_ui_control_frame_t AddFrame(
        UI_CONTROL_RES_REF const & res_ref, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect = NULL);
	void ClearFrame(plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect = NULL);

    
    /* control animation */
    Drow::Animation & createAnim(const char * type_name, const char * name = NULL, plugin_ui_aspect_t aspect = NULL);
    void removeAnimAll(plugin_ui_aspect_t aspect = NULL) { plugin_ui_control_cancel_animations(control(), aspect); }

    template<typename AnimT>
    AnimT & createAnim(const char * name = NULL) { return createAnim(AnimT::TYPE_NAME, name).template as<AnimT>(); }

    template<typename AnimT>
    AnimT & createAnim(plugin_ui_aspect_t aspect) { return createAnim(AnimT::TYPE_NAME, NULL, aspect).template as<AnimT>(); }

    template<typename AnimT>
    AnimT & createAnim(const char * name, plugin_ui_aspect_t aspect) { return createAnim(AnimT::TYPE_NAME, name, aspect).template as<AnimT>(); }

    /* frame animation */
    Drow::Animation & createAnim(plugin_ui_control_frame_t frame, const char * type_name, const char * name = NULL, plugin_ui_aspect_t aspect = NULL);
    void removeAnimAll(plugin_ui_control_frame_t frame, plugin_ui_aspect_t aspect = NULL) { plugin_ui_control_frame_cancel_animations(frame, aspect); }

    template<typename AnimT>
    AnimT & createAnim(plugin_ui_control_frame_t frame, const char * name = NULL) {
        return createAnim(frame, AnimT::TYPE_NAME, name).template as<AnimT>();
    }

    template<typename AnimT>
    AnimT & createAnim(plugin_ui_control_frame_t frame, plugin_ui_aspect_t aspect) {
        return createAnim(frame, AnimT::TYPE_NAME, NULL, aspect).template as<AnimT>();
    }

    template<typename AnimT>
    AnimT & createAnim(plugin_ui_control_frame_t frame, const char * name, plugin_ui_aspect_t aspect) {
        return createAnim(frame, AnimT::TYPE_NAME, name, aspect).template as<AnimT>();
    }
    
	/* background */
	float GetAlpha(void) const { return plugin_ui_control_alpha(control()); }
	void SetAlpha(float alpha) { plugin_ui_control_set_alpha(control(), alpha); }
	ui_color GetColor(void) const { return *plugin_ui_control_effective_color(control()); }
	void SetColor(const ui_color & color) { plugin_ui_control_set_color(control(), (ui_color_t)&color); }
	void SetColor(const char * str_color) { RColor c = color(str_color); plugin_ui_control_set_color(control(), &c); }
    RColor GetGrayColor(void) const { return *plugin_ui_control_gray_color(control()); }
    void SetGrayColor(const RColor& color) { plugin_ui_control_set_gray_color(control(), (ui_color_t)&color); }
	void SetGrayColor(const char * str_color) { RColor c = color(str_color); plugin_ui_control_set_gray_color(control(), &c); }
    
	uint8_t GetDrawAlign(void) const { return plugin_ui_control_draw_align(control()); }
	void SetDrawAlign(uint8_t align) { plugin_ui_control_set_draw_align(control(), align); }
    bool WasDrawColor(void) const { return plugin_ui_control_draw_color(control()) ? true : false; }
	void SetDrawColor(bool flag) { plugin_ui_control_set_draw_color(control(), flag ? 1 : 0); }
    bool WasDrawFrame(void) const { return plugin_ui_control_draw_frame(control()) ? true : false; }
	void SetDrawFrame(bool flag) { plugin_ui_control_set_draw_frame(control(), flag ? 1 : 0); }
    bool WasDrawInner(void) const { return plugin_ui_control_draw_inner(control()) ? true : false; }
	void SetDrawInner(bool flag) { plugin_ui_control_set_draw_inner(control(), flag ? 1 : 0); }

    /*frame*/
    uint8_t frameChanged(void) const { return plugin_ui_control_frame_changed(control()); }
	/*
	animation
	*/
	ui_data_control_anim_t GetShowAnimData(void) const { return plugin_ui_control_find_anim_data(control(), ui_control_anim_type_show); }
	ui_data_control_anim_t GetHideAnimData(void) const { return plugin_ui_control_find_anim_data(control(), ui_control_anim_type_hide); }
	ui_data_control_anim_t GetDeadAnimData(void) const { return plugin_ui_control_find_anim_data(control(), ui_control_anim_type_dead); }
	ui_data_control_anim_t GetDownAnimData(void) const { return plugin_ui_control_find_anim_data(control(), ui_control_anim_type_down); }
	ui_data_control_anim_t GetRiseAnimData(void) const { return plugin_ui_control_find_anim_data(control(), ui_control_anim_type_rise); }
	ui_data_control_anim_t GetUserAnimData(void) const { return plugin_ui_control_find_anim_data(control(), ui_control_anim_type_user); }

	/* layout */
	uint8_t GetAlignHorz(void) const { return plugin_ui_control_align_horz(control()); }
	void SetAlignHorz(uint8_t alignHorz) { plugin_ui_control_set_align_horz(control(), alignHorz); }
	uint8_t GetAlignVert(void) const { return plugin_ui_control_align_vert(control()); }
	void SetAlignVert(uint8_t alignVert) { plugin_ui_control_set_align_vert(control(), alignVert); }

    /*滚动坐标 */
	RVector2 GetScrollRealPT(void) const { return *plugin_ui_control_scroll(control()); }
	float GetScrollRealX(void) const { return plugin_ui_control_scroll(control())->x; }
	float GetScrollRealY(void) const { return plugin_ui_control_scroll(control())->y; }
	void SetScrollRealPT(const RVector2& pt) { plugin_ui_control_set_scroll(control(), (ui_vector_2_t)&pt); }
	void SetScrollRealX(float x) { plugin_ui_control_set_scroll_x(control(), x); }
	void SetScrollRealY(float y) { plugin_ui_control_set_scroll_y(control(), y); }

    /*控件左上角点相关操作 */
	RVector2 GetRenderRealPTAbs(void) const { return * plugin_ui_control_real_pt_abs(control()); } /* 控件左上角屏幕坐标 */
    RVector2 GetRenderRealPT(void) const { return * plugin_ui_control_real_pt_to_p(control()); }/* 控件左上角相对于父控件的坐标 */
	float GetRenderRealX(void) const { return plugin_ui_control_real_pt_to_p(control())->x; }
	float GetRenderRealY(void) const { return plugin_ui_control_real_pt_to_p(control())->y; }
    void SetRenderRealPTAbs( const RVector2& pt );
	void SetRenderRealPT( const RVector2& pt ) { plugin_ui_control_set_render_pt_by_real(control(), (ui_vector_2_t)&pt); }
	void SetRenderRealX( float x ) { plugin_ui_control_set_render_pt_x_by_real(control(), x); }
	void SetRenderRealY( float y ) { plugin_ui_control_set_render_pt_y_by_real(control(), y); }

    float GetRenderRealXNoScreenAdj(void) const { return plugin_ui_control_real_pt_to_p_no_screen_adj(control()).x; }
    float GetRenderRealYNoScreenAdj(void) const { return plugin_ui_control_real_pt_to_p_no_screen_adj(control()).y; }

    /*    左上角点原始数据 */
	RGUIUnitVec2 GetRenderUnitPT(void) const { return *plugin_ui_control_render_pt(control()); } /* 控件左上角缩放计算原始数据 */
	RGUIUnit GetRenderUnitX(void) const { return plugin_ui_control_render_pt(control())->x; }
	RGUIUnit GetRenderUnitY(void) const { return plugin_ui_control_render_pt(control())->y; }
	void SetRenderUnitPT( const RGUIUnit& x, const RGUIUnit& y ) {
        UI_UNIT_VECTOR_2 upt = { { x.k, x.b }, { y.k, y.b } };
        plugin_ui_control_set_render_pt(control(), &upt);
    }
	void SetRenderUnitPT( const RGUIUnitVec2& pt ) {
        UI_UNIT_VECTOR_2 upt = { { pt.x.k, pt.x.b }, { pt.y.k, pt.y.b } };
        plugin_ui_control_set_render_pt(control(), &upt);
    }
	void SetRenderUnitX(const RGUIUnit& x) { UI_UNIT ux = { x.k, x.b }; plugin_ui_control_set_render_pt_x(control(), &ux); }
	void SetRenderUnitY(const RGUIUnit& y) { UI_UNIT uy = { y.k, y.b }; plugin_ui_control_set_render_pt_x(control(), &uy); }

    /*控件大小相关操作 */
	RVector2 GetRenderRealSZAbs(void) const { return * plugin_ui_control_real_sz_abs(control()); }
	float GetRenderRealW(void) const { return plugin_ui_control_real_sz_no_scale(control())->x; }
	float GetRenderRealH(void) const { return plugin_ui_control_real_sz_no_scale(control())->y; }
	RVector2 GetRenderRealSZ(void) const { return * plugin_ui_control_real_sz_no_scale(control()); }
	void SetRenderRealSZ(const RVector2& sz) { plugin_ui_control_set_render_sz_by_real(control(), (ui_vector_2_t)&sz); }
    bool SetRenderRealSZByFrames(void) { return plugin_ui_control_set_render_sz_by_frames(control()) == 0; }
	void SetRenderRealSZToEditor(void) { plugin_ui_control_set_render_sz_by_real(control(), plugin_ui_control_editor_sz(control())); }
	void SetRenderRealW(float w) { plugin_ui_control_set_render_sz_w_by_real(control(), w); }
	void SetRenderRealH(float h) { plugin_ui_control_set_render_sz_h_by_real(control(), h); }

    RVector2 CalcChildRenderRealSZ(ui_data_control_t child_data) const {
        return plugin_ui_control_calc_child_real_sz_no_scale(control(), child_data);
    }

    /*控件大小原始数据 */
    RGUIUnit GetRenderUnitW(void) const { return plugin_ui_control_render_sz(control())->x; }
	RGUIUnit GetRenderUnitH(void) const { return plugin_ui_control_render_sz(control())->y; }
	RGUIUnitVec2 GetRenderUnitSZ(void) const { return *plugin_ui_control_render_sz(control()); }
	void SetRenderUnitSZ(const RGUIUnit& w, const RGUIUnit& h) {
        UI_UNIT_VECTOR_2 usz = { { w.k, w.b }, { h.k, h.b } };
        plugin_ui_control_set_render_sz(control(), &usz);
    }
	void SetRenderUnitSZ(const RGUIUnitVec2& sz) {
        UI_UNIT_VECTOR_2 usz = { { sz.x.k, sz.x.b }, { sz.y.k, sz.y.b } };
        plugin_ui_control_set_render_sz(control(), &usz);
    }
	void SetRenderUnitW(const RGUIUnit& w) { UI_UNIT uw = { w.k, w.b }; plugin_ui_control_set_render_sz_w(control(), &uw); }
	void SetRenderUnitH(const RGUIUnit& h) { UI_UNIT uh = { h.k, h.b }; plugin_ui_control_set_render_sz_h(control(), &uh); }

    /*包含控件区域 */
	RVector2 GetClientRealSZ(void) const { return * plugin_ui_control_client_real_sz(control()); }
    void SetClientRealSZ(const RVector2& sz) { plugin_ui_control_set_client_real_sz(control(), (ui_vector_2_t)&sz); }
	ui_rect GetClientRealPD(void) const { return *plugin_ui_control_client_real_pd(control()); }
    ui_rect GetClientTextRT(void) const;

    /**/
    RVector2 GetEditorRealSZ(void) const { return *plugin_ui_control_editor_sz(control()); }
    
    /*控件包围框 */
	ui_rect GetRenderRealRTAbs(void) const { return plugin_ui_control_real_rt_abs(control()); }
	ui_rect GetRenderRealRT(void) const {
        const RVector2& sz = GetRenderRealSZ();
        ui_rect r = UI_RECT_INITLIZER(0, 0, (int32_t)sz.w, (int32_t)sz.h);
        return r;
    }

    /**/
	ui_rect GetClipedRealRTAbs(void) const { return *plugin_ui_control_cliped_rt_abs(control()); }

	RColor GetRenderRealColor(void) const { return * plugin_ui_control_render_color(control()); }
	RVector2 GetRenderRealScale(void) const { return * plugin_ui_control_render_scale(control()); }
	ui_vector_3 GetRenderRealAngle(void) const { return * plugin_ui_control_render_angle(control()); }
    
	RVector2 GetRenderRealPTRel(const RVector2& absolute) const;
    
	ui_rect GetRenderTextRT(void) const;
	ui_rect GetRenderTextRTAbs(void) const;

	const char * GetUserText(void) const { return plugin_ui_control_user_text(control()); }
	void SetUserText(const char * text) { plugin_ui_control_set_user_text(control(), text); }

    template<typename T>
    T GetUserText(void) {
        return Cpe::Dr::CTypeTraits<T>::from_string(plugin_ui_control_user_text(control()));
    }

    template<typename T>
    T GetUserText(T dft) {
        return Cpe::Dr::CTypeTraits<T>::from_string(plugin_ui_control_user_text(control()), dft);
    }

    template<typename T>
    void SetUserText(T data) {
        SetUserText(Cpe::Dr::CTypeTraits<T>::to_string(app().tmpBuffer(), data));
    }

	virtual void Retext(void);
	void RetextRecur(void);

    virtual void Load(ui_data_control_t control);
	virtual void Update(float deltaTime);

    const char * visibleMsg(uint32_t msg_id) const;
    const char * visibleMsg(uint32_t msg_id, char * args) const;
    const char * visiableTime(uint32_t msg_id, uint32_t t) const;
    const char * visiableTime(uint32_t msg_id, uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t sec, uint8_t min) const;
    const char * visiableTimeDuration(uint32_t msg_id, int time_diff) const;
    const char * visiableTimeDuration(uint32_t msg_id, uint32_t base, uint32_t v) const;

    RColor color(const char * color, ui_color const & dft_color = UI_COLOR_WHITE);
    
protected:
	/* 
	call back
	*/
	//创建&销毁 
	virtual void OnLoadProperty(void);

protected:
	//更新控件 
	//绘制控件 
	virtual void							UpdateSelf						( float deltaTime );

protected:
	//鼠标
    static void on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);

protected:
    int frame_rect_or_static(plugin_ui_control_frame_t frame, ui_rect & rect);
    
    plugin_ui_aspect_t lock_aspect(void) { return plugin_ui_control_lock_aspect(control()); }
    plugin_editor_module_t editor_module(void);
    mem_allocrator_t allocrator(void) { return plugin_ui_control_allocrator(control()); }
    
    void clear_frames(plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect = NULL);
    plugin_editor_editing_t create_editing(plugin_layout_render_t render);

    static int back_res_setter(plugin_ui_control_t control, dr_value_t value);
    static int add_back_res_setter(plugin_ui_control_t control, dr_value_t value);
    static int tail_res_setter(plugin_ui_control_t control, dr_value_t value);
    static int add_tail_res_setter(plugin_ui_control_t control, dr_value_t value);
    static int color_setter(plugin_ui_control_t control, dr_value_t value);
    static int enable_setter(plugin_ui_control_t control, dr_value_t value);
    static int visible_setter(plugin_ui_control_t control, dr_value_t value);
    static int alpha_setter(plugin_ui_control_t control, dr_value_t value);
    static int scale_setter(plugin_ui_control_t control, dr_value_t value);
    static int user_text_setter(plugin_ui_control_t control, dr_value_t value);
    static int user_text_getter(plugin_ui_control_t control, dr_value_t data);
    static int anim_setter(plugin_ui_control_t control, dr_value_t value);
    static int frame_setter(plugin_ui_control_t control, dr_value_t value);
    static void setup(plugin_ui_control_meta_t meta);
    
protected:
	virtual ~RGUIControl(void);
};

#define DROW_UI_DEF_CREATABLE_CONTROL(__type, __id) \
template<> struct RGUIControlTraits<__type> { enum { TYPE_ID = __id }; }

#include "RGUIControl.inl"

#endif
