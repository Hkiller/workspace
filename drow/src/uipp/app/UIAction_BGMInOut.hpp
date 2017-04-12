#ifndef UIPP_APP_ACTION_BGM_IN_OUT_H
#define UIPP_APP_ACTION_BGM_IN_OUT_H
#include "render/utils/ui_percent_decorator.h"
#include "uipp/sprite_2d/System.hpp"
#include "UIActionGen_WithControl.hpp"

namespace UI { namespace App {

class UIAction_BGMInOut : public UIActionGen_WithControl<UIAction_BGMInOut> {
public:
	enum Way {
		Way_In = 1,
		Way_Out = 2,
	};

    UIAction_BGMInOut(Sprite::Fsm::Action & action);
    UIAction_BGMInOut(Sprite::Fsm::Action & action, UIAction_BGMInOut const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

	void setTakeTime(const float take_time) { m_take_time = take_time; }
	void setWay(const char * zoom_way);

	void setDecotator(const char* def);
    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
	ui_percent_decorator m_percent_decorator;
	Way	  m_way;
	float m_runing_time;
    float m_take_time;
    ui_runtime_sound_group_t m_bgm;
};

}}

#endif
