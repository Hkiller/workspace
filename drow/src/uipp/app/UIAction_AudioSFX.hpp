#ifndef UIPP_APP_UIACTION_AUDIOSFX_H
#define UIPP_APP_UIACTION_AUDIOSFX_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"

namespace UI { namespace App {

class UIAction_AudioSFX : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_AudioSFX> {
public:
    UIAction_AudioSFX(Sprite::Fsm::Action & action);
    UIAction_AudioSFX(Sprite::Fsm::Action & action, UIAction_AudioSFX const & o);

    int enter(void);
	void update(float delta);
    void exit(void);

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

    void setRes(const char * res) { m_res = res; }
	void setLoop(uint8_t loop) { m_loop = loop; }
	void setCut(uint8_t cut) { m_cut = cut; }
	

private:
    ::std::string m_res;
	uint8_t	m_loop;
	uint8_t m_cut;
    uint32_t m_audio_id;
};

}}

#endif
