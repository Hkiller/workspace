#include "gdpp/app/Log.hpp"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_sound_group.h"
#include "render/runtime/ui_runtime_sound_playing.h"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "UIAction_TriggerSFX.hpp"

namespace UI { namespace App {

UIAction_TriggerSFX::UIAction_TriggerSFX(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
}

UIAction_TriggerSFX::UIAction_TriggerSFX(Sprite::Fsm::Action & action, UIAction_TriggerSFX const & o)
    : ActionBase(action, o)
    , m_res(o.m_res)
{
}

int UIAction_TriggerSFX::enter(void) {
    ui_runtime_module_t sound_module;
    ui_runtime_sound_group_t sfx;
    
    sound_module = ui_runtime_module_find_nc(app(), NULL);
    if (sound_module == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play sfx no sound module!", entity().id(), entity().name());
        return -1;
    }

    sfx = ui_runtime_sound_group_find(sound_module, "sfx");
    if (sfx == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play sfx no sfx group!", entity().id(), entity().name());
        return -1;
    }

    ui_runtime_sound_playing_t playing = ui_runtime_sound_group_play_by_path(sfx, m_res.c_str(), 0);
    if (playing == NULL) {
        APP_CTX_ERROR(app(), "entity %d(%s): play sfx %s faild!", entity().id(), entity().name(), m_res.c_str());
        return -1;
    }

    return 0;
}

void UIAction_TriggerSFX::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_TriggerSFX>(repo)
        .on_enter(&UIAction_TriggerSFX::enter)
        ;
}

const char * UIAction_TriggerSFX::NAME = "trigger-sfx";

}}

