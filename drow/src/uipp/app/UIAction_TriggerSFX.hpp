#ifndef UIPP_APP_UIACTION_TRIGGERSFX_H
#define UIPP_APP_UIACTION_TRIGGERSFX_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"

namespace UI { namespace App {

class UIAction_TriggerSFX : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_TriggerSFX> {
public:
    UIAction_TriggerSFX(Sprite::Fsm::Action & action);
    UIAction_TriggerSFX(Sprite::Fsm::Action & action, UIAction_TriggerSFX const & o);

    int enter(void);
	void update(float delta);
    void exit(void);

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

    void setRes(const char * res) { m_res = res; }

private:
    ::std::string m_res;
};

}}

#endif
