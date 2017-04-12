#ifndef UIPP_APP_UIACTION_AUDIOBGM_H
#define UIPP_APP_UIACTION_AUDIOBGM_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"

namespace UI { namespace App {

class UIAction_AudioBGM : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_AudioBGM> {
public:
	UIAction_AudioBGM(Sprite::Fsm::Action & action);
	UIAction_AudioBGM(Sprite::Fsm::Action & action, UIAction_AudioBGM const & o);

	int enter(void);
	void exit(void);
    void update(float delta);

	static const char * NAME;
	static void install(Sprite::Fsm::Repository & repo);

	void setRes(const char * res) { m_res = res; }
	void setLoop(uint8_t loop) { m_loop = loop; }
    void setFadeTime(const float fade_time) { m_fade_time = fade_time; }

    
private:
    ui_percent_decorator m_percent_decorator;
	::std::string	m_res;
	uint8_t			m_loop;
	int				m_audio_id;
    float           m_runing_time;
    float           m_fade_time;
    ui_runtime_sound_group_t m_bgm;
};

}}

#endif
