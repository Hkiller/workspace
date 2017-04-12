#ifndef UIPP_APP_ACTION_MASK_CONTROL_H
#define UIPP_APP_ACTION_MASK_CONTROL_H
#include "render/utils/ui_percent_decorator.h"
#include "uipp/sprite_2d/System.hpp"
#include "UIActionGen_WithControl.hpp"

namespace UI { namespace App {

class UIAction_MaskControl : public UIActionGen_WithControl<UIAction_MaskControl> {
public:

    UIAction_MaskControl(Sprite::Fsm::Action & action);
    UIAction_MaskControl(Sprite::Fsm::Action & action, UIAction_MaskControl const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    void setTakeTime(const float take_time) { m_take_time = take_time; }

	void setPolicy(const char * policy) { m_policy = policy; }
	void setTargetControl(const char * control_name);

	void setDecotator(const char* def);
    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

	Sprite::P2D::Pair getControlPos(RGUIControl * control);

private:
	::std::string m_target_cotrol_name;
	ui_percent_decorator m_percent_decorator;
	::std::string	  m_policy;

	Sprite::P2D::Pair m_origin_pos;
	Sprite::P2D::Pair m_target_pos;
	float m_runing_time;
	float m_duration;
    float m_take_time;
};

}}

#endif
