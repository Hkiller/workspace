#ifndef UIPP_APP_PAGE_H
#define UIPP_APP_PAGE_H
#include "RGUIWindow.h"
#include "gdpp/app/Application.hpp"
#include "System.hpp"

namespace UI { namespace App {

class Page : public RGUIWindow {
public:
    Page();
    virtual ~Page();

    void setControlEnable(RGUIControl * control, const char * name, bool is_enable);
    void setControlEnable(const char * name, bool is_enable) { setControlEnable(this, name, is_enable); } 

    void setControlPushed(RGUIControl * control, const char * name, bool is_paushed);
    void setControlPushed(const char * name, bool is_paushed) { setControlPushed(this, name, is_paushed); } 
    
    void setControlVisible(RGUIControl * control, const char * name, bool is_visible);
    void setControlVisible(const char * name, bool is_visible) { setControlVisible(this, name, is_visible); }

    void setControlColor(RGUIControl * control, const char * name, ui_color const & color);
    void setControlColor(const char * name, ui_color const & color) { setControlColor(this, name, color); }
    
    void setLabelText(RGUIControl * control, const char * name, const char * text);
    void setLabelText(const char * name, const char * text) { setLabelText(this, name, text); }
    void setLabelTextColor(RGUIControl * control, const char * name, ui_color const & color);
    void setLabelTextColor(const char * name, ui_color const & color) { setLabelTextColor(this, name, color); }
    void setLabelTextColor(RGUIControl * control, const char * name, const char * color);
    void setLabelTextColor(const char * name, const char * color) { setLabelTextColor(this, name, color); }
    void setLabelTextSize(RGUIControl * control, const char * name, uint8_t font_size);
    void setLabelTextSize(const char * name, uint8_t font_size) { setLabelTextSize(this, name, font_size); }
    
    void setAcceptClick(RGUIControl * control, const char * name, bool accept_click);
    void setAcceptClick(const char * name, bool accept_click) { setAcceptClick(this, name, accept_click); }
    
    template<typename T>
    void setLabelText(RGUIControl * control, const char * name, T text) {
        setLabelText(control, name, Cpe::Dr::CTypeTraits<T>::to_string(app().tmpBuffer(), text));
    }

    template<typename T>
    void setLabelText(const char * name, T text) {
        setLabelText(this, name, Cpe::Dr::CTypeTraits<T>::to_string(app().tmpBuffer(), text));
    }

    void setIndex(RGUIControl * control, const char * name, int index);
    void setIndex(const char * name, int index) { setIndex(this, name, index); }

    void setListCount(RGUIControl * control, const char * name, uint32_t item_count);
    void setListCount(const char * name, uint32_t item_count) { setListCount(this, name, item_count); }

	void setProgressMode(RGUIControl * control, const char * name, uint8_t mode);
	void setProgressMode(const char * name, uint8_t mode) { setProgressMode(this, name, mode); }

    void setProgress(RGUIControl * control, const char * name, float percent, float speed = -1.0f);
    void setProgress(const char * name, float percent, float speed = -1.0f) { setProgress(this, name, percent, speed); }

    void setToProgress(RGUIControl * control, const char * name, float percent);
    void setToProgress(const char * name, float percent) { setToProgress(this, name, percent); }
    
	bool isProgressComplete(RGUIControl * control, const char * name);
	bool isProgressComplete(const char * name) { return isProgressComplete(this, name); }

    void setUserData(RGUIControl * control, const char * data);
    void setUserData(RGUIControl * control, const char * name, const char * data);

	plugin_ui_control_frame_t setBackFrame(RGUIControl * control, const char * name, const char * resource);
	plugin_ui_control_frame_t setBackFrame(const char * name, const char * resource) { return setBackFrame(this, name, resource); }

	plugin_ui_control_frame_t setFloatFrame(RGUIControl * control, const char * name, const char * resource);
	plugin_ui_control_frame_t setFloatFrame(const char * name, const char * resource) { return setFloatFrame(this, name, resource); }

	plugin_ui_control_frame_t setDownFrame(RGUIControl * control, const char * name, const char * resource);
	plugin_ui_control_frame_t setDownFrame(const char * name, const char * resource) { return setDownFrame(this, name, resource); }

	void setBackAndDownFrame(RGUIControl * control, const char * name, const char * resource);
	void setBackAndDownFrame(const char * name, const char * resource) { setBackFrame(this, name, resource);setDownFrame(this, name, resource); }
    
	bool isChildOf(RGUIControl* control, const char * name) const;

	bool isControlNameWith(RGUIControl* control, const char * str) const;

    RGUIControl * findChild(RGUIControl * control, const char * name) { return control->GetChildByPath(name); }
    RGUIControl * findChild(const char * name) { return GetChildByPath(name); }

    ::std::string getText(RGUIControl * control, const char * name);
    ::std::string getText(const char * name) { return getText(this, name); }

    template<typename T>
    T getUserData(RGUIControl * control) {
        return Cpe::Dr::CTypeTraits<T>::from_string(control->GetUserText());
    }

    template<typename T>
    T getUserData(RGUIControl * control, const char * name) {
        RGUIControl * c = findChild(control, name);
        assert(c);
        return getUserData<T>(c);
    }

    template<typename T>
    void setUserData(RGUIControl * control, T const & data) {
        setUserData(control, Cpe::Dr::CTypeTraits<T>::to_string(app().tmpBuffer(), data));
    }

    template<typename T>
    void setUserData(RGUIControl * control, const char * name, T const & data) {
        setUserData(control, name, Cpe::Dr::CTypeTraits<T>::to_string(app().tmpBuffer(), data));
    }

    template<typename T>
    void setUserData(const char * name, T const & data) {
        setUserData(this, name, Cpe::Dr::CTypeTraits<T>::to_string(app().tmpBuffer(), data));
    }

    template<typename T>
    void showPopupPages(T const & data, const char * template_name = NULL) {
        showPopupPages(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    const char * curPhaseName(void);
    void phaseSwitch(const char * phase_name, const char * load_phase_name, dr_data_t data = NULL);
    void phaseReset(void);
    void phaseCall(const char * phase_name, const char * load_phase_name, const char * back_phase_name, dr_data_t data = NULL);
    void phaseBack(void);

    /*state operations*/
    const char * curStateName(void);

    void stateCall(
        const char * state_name, bool suspend_old,
        const char * enter = NULL, const char * leave = NULL, dr_data_t data = NULL,
        plugin_ui_renter_policy_t renter_policy = plugin_ui_renter_skip);
    void stateSwitch(
        const char * state_name, const char * enter = NULL, const char * leave = NULL, dr_data_t data = NULL);
    void stateReset(void);
    void stateBack(void);
    void stateBackTo(const char * state);

    template<typename T>
    void stateCall(
        const char * state_name, T const & data,
        bool suspend_old, const char * enter = NULL, const char * leave = NULL,
        plugin_ui_renter_policy_t renter_policy = plugin_ui_renter_skip)
    {
        dr_data d;
        d.m_data = (void*)&data;
        d.m_size = Cpe::Dr::MetaTraits<T>::data_size(data);
        d.m_meta = Cpe::Dr::MetaTraits<T>::META;
        stateCall(state_name, suspend_old, enter, leave, &d, renter_policy);
    }

    template<typename T>
    void stateSwitch(
        const char * state_name, T const & data,
        const char * enter = NULL, const char * leave = NULL)
    {
        dr_data d;
        d.m_data = (void*)&data;
        d.m_size = Cpe::Dr::MetaTraits<T>::data_size(data);
        d.m_meta = Cpe::Dr::MetaTraits<T>::META;
        stateSwitch(state_name, enter, leave, &d);
    }
};

}}

#endif
