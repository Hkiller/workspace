#ifndef DROW_UI_CONTROL_LABEL_H
#define DROW_UI_CONTROL_LABEL_H
#include "plugin/layout/plugin_layout_font_info.h"
#include "plugin/editor/plugin_editor_types.h"
#include "RGUIControl.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUILabel, ui_control_type_label);

class RGUILabel : public RGUIControl {
public:
	RGUILabel(const char * layout = "basic");

	void SetTextA(const char * text);
	const char * GetTextA(void)const;
	size_t GetTextLenA(void) const;
    
	bool GetTextSL( void ) const;
	void SetTextSL(bool flag);

	uint32_t GetTextKey(void) const { return mTextKey; }
	void SetTextKey(uint32_t key) { if (mTextKey != key) { mTextKey  = key; Retext(); } }

	plugin_layout_align_t GetTextAlign(void) const;
	void SetTextAlign(plugin_layout_align_t text_align);

    plugin_layout_font_id_t font_id(void);
    void set_font_size(uint8_t font_size);
    void adj_font_size(int8_t font_size);
    int set_font_family(const char * font);

    plugin_layout_render_t layout_render(void);

	plugin_layout_font_draw_t GetTextBackDraw(void) const;
	void SetTextBackDraw(const plugin_layout_font_draw & drawInfo);

    virtual void Load( ui_data_control_t control);
    void Load(UI_CONTROL_TEXT const & data);

	virtual void Retext(void);

	void SetFontSize(uint8_t font_size) { set_font_size(font_size); }

	void SetNormalTextAlpha(float alpha);
    
    uint8_t WasAutoLayout(void) const;
    void SetAutoLayout(uint8_t auto_layout);
    void SetLayoutSrc(void);
    
    static int text_setter(plugin_ui_control_t control, dr_value_t value);
    static int text_id_setter(plugin_ui_control_t control, dr_value_t value);
    static int text_font_size_setter(plugin_ui_control_t control, dr_value_t value);
    static int text_font_family_setter(plugin_ui_control_t control, dr_value_t value);
    static int text_alpha_setter(plugin_ui_control_t control, dr_value_t value);
    static void setup(plugin_ui_control_meta_t meta);

    ui_rect GetTextBoundRT(void);
    bool PastFromClipboard(void);
    bool CopyToClipboard(void);
    
protected:
	virtual void OnTextChanged();
    void update_layout_sz(void);
    
    uint32_t mTextKey;
    plugin_ui_control_frame_t m_text_frame;

protected:
    static void update_layout_font(plugin_ui_control_frame_t layout_frame, UI_FONT const & font_info, UI_FONT_DROW const & font_draw);
    static void update_layout_align(plugin_ui_control_frame_t layout_frame, plugin_layout_align_t align);
    static void update_layout_draw(plugin_ui_control_frame_t layout_frame, plugin_layout_font_draw_t font_draw);
    static void update_layout_sz(plugin_ui_control_frame_t layout_frame, ui_vector_2_t sz);

    static plugin_layout_layout_basic_t layout_basic(plugin_ui_control_frame_t layout_frame);
    static plugin_layout_layout_rich_t layout_rich(plugin_ui_control_frame_t layout_frame);
    static plugin_layout_render_t layout_render(plugin_ui_control_frame_t layout_frame);

    static void cvt_font_draw(plugin_layout_font_draw & o, UI_FONT_DROW const & i);
    static void cvt_font_id(plugin_layout_font_id & o, UI_FONT const & i, UI_FONT_DROW const & font_draw);
    
    plugin_ui_control_frame_t create_text_frame(const char * layout, plugin_ui_control_frame_usage_t usage);
    
	virtual ~RGUILabel(void);
    
    friend class RGUIControlRepo; 
};

#endif
