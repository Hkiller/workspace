#ifndef UIPP_APP_ACTION_PROGRESSBAR_H
#define UIPP_APP_ACTION_PROGRESSBAR_H
#include "render/utils/ui_percent_decorator.h"
#include "UIActionGen_WithControl.hpp"

namespace UI { namespace App {

class UIAction_UpdateProgress
    : public UIActionGen_WithControl<UIAction_UpdateProgress>
{
public:
    UIAction_UpdateProgress(Sprite::Fsm::Action & action);
    UIAction_UpdateProgress(Sprite::Fsm::Action & action, UIAction_UpdateProgress const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

	void setBindingValue(const char * bindingValue) { m_binding_value = bindingValue; }

	void setSpeed(const float speed) { m_speed = speed; }

	void setAlphaMove(bool alphaMove) { m_alphaMove = alphaMove; }
	
    void setTakeTime(const float take_time) { m_take_time = take_time; }

	void setMode(RGUIProgressBar * progressBar);

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
    void onValueUpdated(void);
    void startUpdate(void);

    /*config*/
	::std::string  m_binding_value;
	float          m_speed;
    float          m_take_time;

    /*runing data*/
	float          m_target_progress;
	bool m_alphaMove;
};

}}

#endif
