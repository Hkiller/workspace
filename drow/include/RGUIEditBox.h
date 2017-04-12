#ifndef DROW_UI_CONTROL_EDITBOX_H
#define DROW_UI_CONTROL_EDITBOX_H
#include <string>
#include "RGUILabel.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIEditBox, ui_control_type_edit_box);

class RGUIEditBox : public RGUILabel {
public:
	RGUIEditBox();

	bool WasMaxLength(void) const;
	uint32_t GetMaxLength(void) const;
	void SetMaxLength(uint32_t length);
	
	bool WasPassword(void) const;
	void SetPassword(bool flag);

	bool WasReadOnly(void) const;
	void SetReadOnly(bool flag);

	bool WasNumberOnly(void) const;
	void SetNumberOnly(bool flag);

	const ::std::string& GetHintText(void) const;
	void SetHintText(const ::std::string& text);

	plugin_layout_font_draw_t GetHintTextDraw(void) const;
	void SetHintTextDraw(const plugin_layout_font_draw& draw);

    virtual void Load(ui_data_control_t control);
    void Load(UI_CONTROL_EDITOR const & data);

	virtual void Update(float deltaTime);

    bool WasRenderHint(void) const { return m_is_rending_hit ? true : false; }
    void SetRenderHint(bool flag);

protected:
	virtual void OnTextChanged();
    static void on_lost_focus(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);

    plugin_editor_editing_t editing(void);

    plugin_editor_editing_t startEditing(void);
    void stopEditing(void);

protected:
	uint32_t		mMaxLength;
	bool		mPassword;
	bool		mReadOnly;
	bool		mNumberOnly;

    uint8_t m_is_rending_hit;
	::std::string mHintText;
	plugin_layout_font_draw	m_saved_font_draw;

protected:
	virtual ~RGUIEditBox( void );
    friend class RGUIControlRepo; 
};

#include "RGUIEditBox.inl"

#endif
