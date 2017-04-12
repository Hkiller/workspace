#include "gdpp/app/Log.hpp"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_sound_group.h"
#include "render/runtime/ui_runtime_sound_playing.h"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "UIAction_AudioSFX.hpp"

namespace UI { namespace App {

UIAction_AudioSFX::UIAction_AudioSFX(Sprite::Fsm::Action & action)
    : ActionBase(action)
	, m_loop(0)
	, m_cut(1)
    , m_audio_id(-1)
{
}

UIAction_AudioSFX::UIAction_AudioSFX(Sprite::Fsm::Action & action, UIAction_AudioSFX const & o)
    : ActionBase(action, o)
    , m_res(o.m_res)
	, m_loop(o.m_loop)
	, m_cut(o.m_cut)
    , m_audio_id(-1)
{
}

int UIAction_AudioSFX::enter(void) {
    ui_runtime_module_t module;
    ui_runtime_sound_group_t sfx;
    
    module = ui_runtime_module_find_nc(app(), NULL);
    if (module == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play sfx no sound module!", entity().id(), entity().name());
        return -1;
    }

    sfx = ui_runtime_sound_group_find(module, "sfx");
    if (sfx == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play sfx no sfx group!", entity().id(), entity().name());
        return -1;
    }

    ui_runtime_sound_playing_t playing = ui_runtime_sound_group_play_by_path(sfx, m_res.c_str(), 0);
    if (playing == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play sfx %s faild!", entity().id(), entity().name(), m_res.c_str());
        return -1;
    }

    m_audio_id = ui_runtime_sound_playing_id(playing);

	if (lifeCircle() == ui_sprite_fsm_action_life_circle_working) {
		startUpdate();
	}

    return 0;
}

void UIAction_AudioSFX::update(float delta) {
    ui_runtime_module_t module = ui_runtime_module_find_nc(app(), NULL);
    if (module == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play sfx: update: no sound module!", entity().id(), entity().name());
        stopUpdate();
        return;
    }

    if (ui_runtime_sound_playing_find_by_id(module, m_audio_id) == NULL) {
        stopUpdate();
    }
}

void UIAction_AudioSFX::exit(void) {
    ui_runtime_module_t module = ui_runtime_module_find_nc(app(), NULL);
    if (module == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play sfx: update: no sound module!", entity().id(), entity().name());
        stopUpdate();
        return;
    }

    ui_runtime_sound_playing_t playing = ui_runtime_sound_playing_find_by_id(module, m_audio_id);
    if (playing) {
        ui_runtime_sound_playing_free(playing);
    }
}

void UIAction_AudioSFX::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_AudioSFX>(repo)
        .on_enter(&UIAction_AudioSFX::enter)
        .on_exit(&UIAction_AudioSFX::exit)
		.on_update(&UIAction_AudioSFX::update)
        ;
}

const char * UIAction_AudioSFX::NAME = "play-sfx";

}}

