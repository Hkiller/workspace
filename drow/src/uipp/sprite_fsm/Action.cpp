#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_fsm/Action.hpp"

namespace UI { namespace Sprite { namespace Fsm {

void Action::startUpdate(void) {
	if (ui_sprite_fsm_action_start_update(*this) != 0) {
        APP_CTX_THROW_EXCEPTION(
            world().app(),
            ::std::runtime_error,
            "ui_sprite_fsm_action start update fail!");			
	}
}

}}}
