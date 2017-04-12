#ifndef UIPP_APP_ACTIONGEN_WITH_CONTROL_H
#define UIPP_APP_ACTIONGEN_WITH_CONTROL_H
#include "RGUIControl.h"
#include "RGUIProgressBar.h"
#include "RGUIPictureCondition.h"
#include "plugin/ui/plugin_ui_page.h"
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/MemBuffer.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"

namespace UI { namespace App {

class UIActionGen_WithControl_Base {
public:
    void setControlName(const char * control_name);
    const char * controlName(void) const { return m_control_name.c_str(); }

    plugin_ui_page_t findPage(const char * control_name = NULL, const char * * name_start = NULL, mem_buffer_t buffer = NULL);
    RGUIControl * findTargetControl(const char * control_name = NULL);

protected:
    virtual UI::Sprite::Fsm::Action & action(void) = 0;
    
private:
    ::std::string m_control_name;
};

template<typename OuterT>
class UIActionGen_WithControl
    : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, OuterT>
    , public UIActionGen_WithControl_Base
{
public:
    typedef UIActionGen_WithControl ActionBase;
    
    UIActionGen_WithControl(Sprite::Fsm::Action & action)
        : Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, OuterT>(action)
    {
    }
    
    UIActionGen_WithControl(Sprite::Fsm::Action & action, UIActionGen_WithControl const & o)
        : Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, OuterT>(action)
        , UIActionGen_WithControl_Base(o)
    {
    }

protected:
    bool calcBool(const char * def, dr_data_source_t data_source = NULL);
    float calcFloat(const char * def, dr_data_source_t data_source = NULL);
    uint32_t calcUInt32(const char * def, dr_data_source_t data_source = NULL);
    int32_t calcInt32(const char * def, dr_data_source_t data_source = NULL);
    const char * calcString(const char * def, mem_buffer_t buff, dr_data_source_t data_source = NULL);

    virtual UI::Sprite::Fsm::Action & action(void) { return Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, OuterT>::action(); }
};


template<typename OuterT>
bool UIActionGen_WithControl<OuterT>::calcBool(const char * def, dr_data_source_t data_source) {
    plugin_ui_page_t page = findPage();
    assert(page);
    
    dr_data_source data_source_buf;
    if (plugin_ui_page_data(page)) {
        data_source_buf.m_data.m_meta = plugin_ui_page_data_meta(page);
        data_source_buf.m_data.m_data = plugin_ui_page_data(page);
        data_source_buf.m_data.m_size = plugin_ui_page_data_size(page);
        data_source_buf.m_next = data_source;
        data_source = &data_source_buf;
    }

    return Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, OuterT>::calcBool(def, data_source);
}

template<typename OuterT>
float UIActionGen_WithControl<OuterT>::calcFloat(const char * def, dr_data_source_t data_source) {
    plugin_ui_page_t page = findPage();
    assert(page);
    
    dr_data_source data_source_buf;

    if (plugin_ui_page_data(page)) {
        data_source_buf.m_data.m_meta = plugin_ui_page_data_meta(page);
        data_source_buf.m_data.m_data = plugin_ui_page_data(page);
        data_source_buf.m_data.m_size = plugin_ui_page_data_size(page);
        data_source_buf.m_next = data_source;
        data_source = &data_source_buf;
    }

    return Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, OuterT>::calcFloat(def, data_source);
}

template<typename OuterT>
uint32_t UIActionGen_WithControl<OuterT>::calcUInt32(const char * def, dr_data_source_t data_source) {
    plugin_ui_page_t page = findPage();
    assert(page);
    
    dr_data_source data_source_buf;
    if (plugin_ui_page_data(page)) {
        data_source_buf.m_data.m_meta = plugin_ui_page_data_meta(page);
        data_source_buf.m_data.m_data = plugin_ui_page_data(page);
        data_source_buf.m_data.m_size = plugin_ui_page_data_size(page);
        data_source_buf.m_next = data_source;
        data_source = &data_source_buf;
    }

    return Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, OuterT>::calcUInt32(def, data_source);
}

template<typename OuterT>
int32_t UIActionGen_WithControl<OuterT>::calcInt32(const char * def, dr_data_source_t data_source) {
    plugin_ui_page_t page = findPage();
    assert(page);
    
    dr_data_source data_source_buf;
    if (plugin_ui_page_data(page)) {
        data_source_buf.m_data.m_meta = plugin_ui_page_data_meta(page);
        data_source_buf.m_data.m_data = plugin_ui_page_data(page);
        data_source_buf.m_data.m_size = plugin_ui_page_data_size(page);
        data_source_buf.m_next = data_source;
        data_source = &data_source_buf;
    }
        
    return Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, OuterT>::calcInt32(def, data_source);
}

template<typename OuterT>
const char * UIActionGen_WithControl<OuterT>::calcString(const char * def, mem_buffer_t buff, dr_data_source_t data_source) {
    plugin_ui_page_t page = findPage();
    assert(page);
    
    dr_data_source data_source_buf;
    if (plugin_ui_page_data(page)) {
        data_source_buf.m_data.m_meta = plugin_ui_page_data_meta(page);
        data_source_buf.m_data.m_data = plugin_ui_page_data(page);
        data_source_buf.m_data.m_size = plugin_ui_page_data_size(page);
        data_source_buf.m_next = data_source;
        data_source = &data_source_buf;
    }
        
    return Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, OuterT>::calcString(def, buff, data_source);
}

}}

#endif
