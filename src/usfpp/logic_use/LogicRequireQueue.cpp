#include <stdexcept>
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "usfpp/logic/LogicOpManager.hpp"
#include "usfpp/logic_use/LogicRequireQueue.hpp"

namespace Usf { namespace Logic {

LogicRequireQueue::LogicRequireQueue(Gd::App::Application & app, const char * name, uint32_t binding_capacity)
    : m_reqruie_queue(logic_require_queue_create(app, app.allocrator(), app.em(), name, LogicOpManager::instance(app), binding_capacity))
{
    if (m_reqruie_queue == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "%s: create queue fail", name);
    }
}

LogicRequireQueue::LogicRequireQueue(Gd::App::Application & app, const char * name, LogicOpManager & logicManager, uint32_t binding_capacity)
    : m_reqruie_queue(logic_require_queue_create(app, app.allocrator(), app.em(), name, logicManager, binding_capacity))
{
    if (m_reqruie_queue == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "%s: create queue fail", name);
    }
}

LogicRequireQueue::~LogicRequireQueue() {
    logic_require_queue_free(m_reqruie_queue);
}

void LogicRequireQueue::add(logic_require_id_t id) {
    if (logic_require_queue_add(m_reqruie_queue, id, NULL, 0) != 0) {
        throw ::std::runtime_error("add require fail");
    }
}

}}
