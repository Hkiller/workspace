#include "gdpp/app/Log.hpp"
#include "render/utils/ui_percent_decorator.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_sound_group.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "cpe/pal/pal_strings.h"
#include "UIAction_AudioBGM.hpp"

namespace UI { namespace App {

UIAction_AudioBGM::UIAction_AudioBGM(Sprite::Fsm::Action & action)
	: ActionBase(action)
	, m_loop(0)
	, m_audio_id(-1)
{
    bzero(&m_percent_decorator, sizeof(m_percent_decorator));
}

UIAction_AudioBGM::UIAction_AudioBGM(Sprite::Fsm::Action & action, UIAction_AudioBGM const & o)
	: ActionBase(action, o)
    , m_percent_decorator(o.m_percent_decorator)
	, m_res(o.m_res)
	, m_loop(o.m_loop)
	, m_audio_id(o.m_audio_id)
    , m_fade_time(o.m_fade_time)
{
}

int UIAction_AudioBGM::enter(void) {
    m_runing_time = 0.0f;
    ui_runtime_module_t sound_module;
    sound_module = ui_runtime_module_find_nc(app(), NULL);
    if (sound_module == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play bgm no sound module!", entity().id(), entity().name());
        return -1;
    }

    m_bgm = ui_runtime_sound_group_find(sound_module, "bgm");
    if (m_bgm == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play bgm no bgm group!", entity().id(), entity().name());
        return -1;
    }
    
    if (ui_runtime_sound_group_play_by_path(m_bgm, m_res.c_str(), 1) == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play bgm %s faild!", entity().id(), entity().name(), m_res.c_str());
        return -1;
    }

    if(m_fade_time > 0.0f) {
        startUpdate();
    }else {
        ui_runtime_sound_group_set_volumn(m_bgm, 1.0f);
    }

	return 0;
}

void UIAction_AudioBGM::update(float delta) {
    m_runing_time += delta;
    float percent = m_runing_time >= m_fade_time ? 1.0f : m_runing_time / m_fade_time;

    percent = ui_percent_decorator_decorate(&m_percent_decorator, percent);


    ui_runtime_sound_group_set_volumn(m_bgm, percent);

    if (m_runing_time >= m_fade_time) {
        stopUpdate();
    }
}

void UIAction_AudioBGM::exit(void) {
}

void UIAction_AudioBGM::install(Sprite::Fsm::Repository & repo) {
	Sprite::Fsm::ActionReg<UIAction_AudioBGM>(repo)
		.on_enter(&UIAction_AudioBGM::enter)
		.on_exit(&UIAction_AudioBGM::exit)
        .on_update(&UIAction_AudioBGM::update)
		;
}

const char * UIAction_AudioBGM::NAME = "play-bgm";

}}

