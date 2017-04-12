#include <stdexcept>
#include "cpe/tl/tl_action.h"
#include "cpe/utils/stream_buffer.h"
#include "gdpp/evt/Event.hpp"

namespace Gd { namespace Evt {

void Event::setTarget(const char * target) {
    if (gd_evt_set_target(*this, target) != 0) {
        throw ::std::runtime_error("Gd::Evt::Event set target fail!");
    }
}

Event & Event::_cast(gd_evt_t evt) {
    if (evt == NULL) {
        throw ::std::runtime_error("cast to Gd::Evt::Event, input evt is null!");
    }

    return *(Event*)evt;
}

Event & Event::_cast(tl_event_t tl_evt) {
    return Event::_cast(gd_evt_cvt(tl_evt));
}

Event * Event::clone(mem_allocrator_t alloc) const {
    tl_event_t from_evt = tl_event_from_data((void*)(gd_evt_t)*this);

    tl_event_t new_evt = tl_event_clone(from_evt, alloc);
    if (new_evt == NULL) {
        throw ::std::runtime_error("clone Gd::Evt::Event, return evt is null!");
    }

    return (Event*)gd_evt_cvt(new_evt);
}

void Event::destory(void) {
    tl_event_t tl_evt = tl_event_from_data((void*)(gd_evt_t)this);
    tl_event_free(tl_evt);
}

const char * Event::dump(mem_buffer_t buffer) const {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    mem_buffer_clear_data(buffer);
    dump((write_stream_t)&stream);
    stream_putc((write_stream_t)&stream, 0);
    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

}}
