#ifndef UIPP_APP_ACTION_HIDECONTROL_H
#define UIPP_APP_ACTION_HIDECONTROL_H
#include "UIActionGen_WithControl.hpp"

namespace UI { namespace App {

class UIAction_HideControl : public UIActionGen_WithControl<UIAction_HideControl> {
public:
    UIAction_HideControl(Sprite::Fsm::Action & action);
    UIAction_HideControl(Sprite::Fsm::Action & action, UIAction_HideControl const & o);

    int enter(void);
	void exit(void);
    void update(float delta);

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
	bool m_saved_visible_state;
};

}}

#endif
