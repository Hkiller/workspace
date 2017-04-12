#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "UIAction_BGMInOut.hpp"
#include "render/utils/ui_percent_decorator.h"
#include "cpe/pal/pal_strings.h"
#include "render/runtime/ui_runtime_sound_group.h"

namespace UI { namespace App {

UIAction_BGMInOut::UIAction_BGMInOut(Sprite::Fsm::Action & action)
    : ActionBase(action)
	, m_way(Way_In)
{
	bzero(&m_percent_decorator, sizeof(m_percent_decorator));
}

UIAction_BGMInOut::UIAction_BGMInOut(Sprite::Fsm::Action & action, UIAction_BGMInOut const & o)
    : ActionBase(action, o)
	, m_percent_decorator(o.m_percent_decorator)
	, m_way(o.m_way)
    , m_take_time(o.m_take_time)
{
}

void UIAction_BGMInOut::setWay(const char * zoom_way) {
	if (strcmp(zoom_way, "in") == 0) {
		m_way = Way_In;
	}
	else if (strcmp(zoom_way, "out") == 0) {
		m_way = Way_Out;
	}
	else {
		APP_CTX_THROW_EXCEPTION(
			app(), ::std::runtime_error, "entity %d(%s): %s: set bgm volum way %s error",
			entity().id(), entity().name(), name(), zoom_way);
	}
}

int UIAction_BGMInOut::enter(void) {
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

    if(m_way == Way_In){
        ui_runtime_sound_group_set_volumn(m_bgm, 0);
    }

	startUpdate();

    return 0;
}

void UIAction_BGMInOut::update(float delta) {
    m_runing_time += delta;
    float percent = m_runing_time >= m_take_time ? 1.0f : m_runing_time / m_take_time;

    if(m_way == Way_In){
        percent = ui_percent_decorator_decorate(&m_percent_decorator, percent);
    }
    else if(m_way == Way_Out){
        percent = 1.0f - ui_percent_decorator_decorate(&m_percent_decorator, percent);
    }
    else{
        APP_CTX_ERROR(
            app(), "entity %d(%s): %s: update: bgm volum way %d eror",
            entity().id(), entity().name(), name(), m_way);
        return;
    }

    ui_runtime_sound_group_set_volumn(m_bgm, percent);

    if (m_runing_time >= m_take_time) {
        stopUpdate();
    }
}

void UIAction_BGMInOut::exit(void) {
	if(m_way == Way_Out){
        ui_runtime_sound_group_set_volumn(m_bgm, 0);
    }
}

void UIAction_BGMInOut::setDecotator(const char* def){
	if(ui_percent_decorator_setup(&m_percent_decorator,def, app().em()) != 0){
		APP_CTX_THROW_EXCEPTION(
			app(), ::std::runtime_error,
			"entity %d(%s): %s: set decorate %s fail",
			entity().id(), entity().name(), name(), def);
	}
}

void UIAction_BGMInOut::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_BGMInOut>(repo)
        .on_enter(&UIAction_BGMInOut::enter)
        .on_exit(&UIAction_BGMInOut::exit)
        .on_update(&UIAction_BGMInOut::update)
        ;
}

const char * UIAction_BGMInOut::NAME = "bgm-in-out";

}}

