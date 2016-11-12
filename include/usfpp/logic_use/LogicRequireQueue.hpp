#ifndef USFPP_LOGIC_USE_REQUIRE_QUEUE_H
#define USFPP_LOGIC_USE_REQUIRE_QUEUE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usf/logic_use/logic_require_queue.h"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicRequireQueue : public Cpe::Utils::Noncopyable {
public:
    operator logic_require_queue_t () const { return m_reqruie_queue; }

    LogicRequireQueue(Gd::App::Application & app, const char * name, uint32_t binding_capacity = 0);
    LogicRequireQueue(Gd::App::Application & app, const char * name, LogicOpManager & logicManager, uint32_t binding_capacity = 0);
    ~LogicRequireQueue();

    int count(void) const { return logic_require_queue_require_count(*this); }

    void add(logic_require_id_t id);
    bool remove(logic_require_id_t id) { return logic_require_queue_remove(*this, id, NULL, NULL) == 0 ? false : true; }
    logic_require_t retrieve(logic_require_id_t id) { return logic_require_queue_remove_get(*this, id, NULL, NULL); }

    void notifyAll(int32_t err) { logic_require_queue_notify_all(*this, err); }
    void cancelAll(void) { logic_require_queue_cancel_all(*this); }

private:
    logic_require_queue_t m_reqruie_queue;
};

}}

#endif
