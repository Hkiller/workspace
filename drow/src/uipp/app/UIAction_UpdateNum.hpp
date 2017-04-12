#ifndef UIPP_APP_ACTION_UPDATENUM_H
#define UIPP_APP_ACTION_UPDATENUM_H
#include "render/utils/ui_percent_decorator.h"
#include "UIActionGen_WithControl.hpp"

namespace UI { namespace App {

class UIAction_UpdateNum
    : public UIActionGen_WithControl<UIAction_UpdateNum>
{
public:
    UIAction_UpdateNum(Sprite::Fsm::Action & action);
    UIAction_UpdateNum(Sprite::Fsm::Action & action, UIAction_UpdateNum const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

	void setBindingValue(const char * bindingValue) { m_binding_value = bindingValue; }
    void setInitValue(const char * initValue) { m_init_value = initValue; }

	void setTakeTime(const float take_time) {  m_take_time = take_time; }

	void setDecotator(const char* def);

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
    void onBindingValueChanged(void);
    void onInitValue(void);
    void startUpdate(int32_t target_num);
    void setValue(int32_t value);

    /*config*/
	ui_percent_decorator m_percent_decorator;
	float		 m_take_time;
	::std::string m_binding_value;
    ::std::string m_init_value;

    /*runtime*/
	int32_t m_target_num;
	int32_t	m_origin_num;
	float m_runing_time;
};

}}

#endif
