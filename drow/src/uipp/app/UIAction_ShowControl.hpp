#ifndef UIPP_APP_ACTION_SHOWCONTROL_H
#define UIPP_APP_ACTION_SHOWCONTROL_H
#include "cpepp/utils/ObjRef.hpp"
#include "UIActionGen_WithControl.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIAction_ShowControl : public UIActionGen_WithControl<UIAction_ShowControl> {
public:
    UIAction_ShowControl(Sprite::Fsm::Action & action);
    UIAction_ShowControl(Sprite::Fsm::Action & action, UIAction_ShowControl const & o);

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
