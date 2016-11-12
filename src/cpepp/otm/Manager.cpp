#include <cassert>
#include <stdexcept>
#include "cpe/otm/otm_timer.h"
#include "cpepp/otm/Memo.hpp"
#include "cpepp/otm/Timer.hpp"
#include "cpepp/otm/TimerProcessor.hpp"
#include "cpepp/otm/Manager.hpp"

namespace Cpe { namespace Otm {

class DummyTimerContext;
typedef TimerProcessor<DummyTimerContext> DummyTimerProcessor;
typedef Manager<DummyTimerContext> DummyManager;

struct TimerData {
    error_monitor_t m_em;
    DummyTimerProcessor * m_real_responser;
    DummyTimerProcessor * m_use_responser;
    DummyManager::Fun m_fun;
};

void otm_process_adp(otm_timer_t timer, otm_memo_t memo, uint32_t cur_exec_time_s, void * obj_ctx) {
     TimerData * data = (TimerData*)otm_timer_data(timer);

     try {
         (data->m_use_responser->*(data->m_fun))(
             *(Timer*)timer,
             *(Memo*)memo,
             cur_exec_time_s,
             *(DummyTimerContext*)obj_ctx);
     }
     catch(::std::exception const & e) {
         CPE_ERROR(
             data->m_em, "otm timer %d(%s): %s!",
             otm_timer_id(timer), otm_timer_name(timer), e.what());
     }
     catch(...) {
         CPE_ERROR(
             data->m_em, "otm timer %d(%s): catch unknown exception!",
             otm_timer_id(timer), otm_timer_name(timer));
     }
}

Timer &
ManagerBase::registerTimer(
        otm_timer_id_t id,
        const char * name,
        void * realResponser, void * fun_addr, size_t fun_size,
        uint32_t span_s
#ifdef _MSC_VER
        , void * useResponser
#endif
        )
{
    assert(fun_size == sizeof(DummyManager::Fun));

    otm_timer_t timer =
        otm_timer_create(
            *this, id, name, span_s, sizeof(TimerData),
            otm_process_adp);
    if (timer == NULL) {
        if (otm_timer_find(*this, id)) {
            throw ::std::runtime_error("otm create timer fail, id duplicate!"); 
        }
        else {
            throw ::std::runtime_error("otm create timer fail, unknown error!"); 
        }
    }

    TimerData * data = (TimerData*)otm_timer_data(timer);

    data->m_em = otm_manage_em(*this);
    data->m_real_responser = (DummyTimerProcessor*)realResponser;
#ifdef _MSC_VER
    data->m_use_responser = (DummyTimerProcessor*)useResponser;
#else
    data->m_use_responser = (DummyTimerProcessor*)realResponser;
#endif
    memcpy(&data->m_fun, fun_addr, fun_size);

    return *(Timer*)timer;
}

void ManagerBase::unregisterTimer(otm_timer_id_t id) {
    otm_timer_t timer = otm_timer_find(*this, id);
    if (timer) {
        otm_timer_free(timer);
    }
}

void ManagerBase::unregisterTimer(void const * processor) {
    struct otm_timer_it it;
    otm_manage_timers(*this, &it);

    otm_timer_t timer = otm_timer_next(&it);
    while(timer) {
        otm_timer_t next = otm_timer_next(&it);
        struct TimerData * data = (struct TimerData *)otm_timer_data(timer);

        if (data->m_real_responser == processor) {
            otm_timer_free(timer);
        }

        timer = next;
    }
}

ManagerBase & ManagerBase::_cast(otm_manage_t otm) {
    if (otm == NULL) {
        throw ::std::runtime_error("otm is NULL!"); 
    }

    return *reinterpret_cast<ManagerBase*>(otm);
}

}}
