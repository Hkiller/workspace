#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/World.hpp"
#include "cpepp/utils/MemBuffer.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_action_fsm.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "uipp/sprite_fsm/Fsm.hpp"
#include "uipp/sprite_fsm/ActionVisitor.hpp"

namespace UI { namespace Sprite { namespace Fsm {

State & Fsm::createState(const char * name) {
    ui_sprite_fsm_state_t state = ui_sprite_fsm_state_create(*this, name);
    if (state == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().app(), ::std::runtime_error, "Fsm create %s fail!", name);			
    }

    return *(State*)state;
}

void Fsm::copy(Fsm const & from) {
    if (ui_sprite_fsm_ins_copy(*this, from) != 0) {
        APP_CTX_THROW_EXCEPTION(world().app(), ::std::runtime_error, "Fsm copy fail!");
    }
}

void Fsm::setDefaultState(const char * name) {
    if (ui_sprite_fsm_set_default_state(*this, name) != 0) {
        APP_CTX_THROW_EXCEPTION(world().app(), ::std::runtime_error, "set default state %s fail!", name);
    }
}

void Fsm::setDefaultCallState(const char * name) {
    if (ui_sprite_fsm_set_default_call_state(*this, name) != 0) {
        APP_CTX_THROW_EXCEPTION(world().app(), ::std::runtime_error, "set default call state %s fail!", name);
    }
}

const char * Fsm::path(Cpe::Utils::MemBuffer & buff) const {
    return ui_sprite_fsm_dump_path(*this, buff);
}

static void call_visitor(void * ctx, ui_sprite_fsm_action_t action) {
    try {
        ((ActionVisitor *)ctx)->onAction(*(Action *)action);
    }
    catch(...) {
    }
}

void Fsm::visitActions(ActionVisitor & visitor) {
    ui_sprite_fsm_ins_visit_actions(*this, call_visitor, &visitor);
}

const char * ComponentFsm::NAME = UI_SPRITE_FSM_COMPONENT_FSM_NAME;

const char * ActionFsm::NAME = UI_SPRITE_FSM_ACTION_FSM_NAME;

void ActionFsm::setData(dr_data_t data) {
    if (ui_sprite_fsm_action_fsm_set_data(*this, data) != 0) {
        APP_CTX_THROW_EXCEPTION(world().app(), ::std::runtime_error, "set action fsm data fail!");
    }
}

ActionVisitor::~ActionVisitor() {
}

}}}
