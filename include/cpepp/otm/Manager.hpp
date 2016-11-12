#ifndef CPEPP_OTM_MANAGER_H
#define CPEPP_OTM_MANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/otm/otm_manage.h"
#include "TimerProcessor.hpp"
#include "MemoBuf.hpp"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Otm {

class ManagerBase : public Cpe::Utils::SimulateObject {
public:
    operator otm_manage_t (void) const { return (otm_manage_t)(this); }

    void init(uint32_t cur_time_s, otm_memo_t memo, size_t memo_capacitiy) {
        otm_manage_buf_init(*this, cur_time_s, memo, memo_capacitiy);
    }

    void tick(uint32_t cur_time_s, void * obj_ctx, otm_memo_t memo, size_t memo_capacitiy) {
        otm_manage_tick(*this, cur_time_s, obj_ctx, memo, memo_capacitiy);
    }

    void enable(otm_timer_id_t id, uint32_t cur_time_s, otm_memo_t memo_buf, size_t memo_capacitiy, uint32_t first_exec_span_s = 0) {
        otm_manage_enable(*this, id, cur_time_s, first_exec_span_s, memo_buf, memo_capacitiy);
    }

    void disable(otm_timer_id_t id, otm_memo_t memo_buf, size_t memo_capacitiy) {
        otm_manage_disable(*this, id, memo_buf, memo_capacitiy);
    }

    template<size_t capacity>
    void init(uint32_t cur_time_s, MemoBuf<capacity> & buf) {
        init(cur_time_s, buf, capacity);
    }

    template<size_t capacity>
    void enable(otm_timer_id_t id, uint32_t cur_time_s, MemoBuf<capacity> & buf, uint32_t first_exec_span_s = 0) { 
        enable(id, cur_time_s, buf, capacity, first_exec_span_s);
    }

    template<size_t capacity>
    void disable(otm_timer_id_t id, MemoBuf<capacity> & buf) { 
        disable(id, buf, capacity);
    }


	void unregisterTimer(otm_timer_id_t id);

    static ManagerBase & _cast(otm_manage_t otm);

protected:
    /*VC编译器处理成员函数地址时有错误，没有生成垫片函数，所以为了正确调用函数指针，必须直接把传入T类型的对象绑定在调用的对象上
      所以传入的realResponser为真实的Responser地址，而useResponser是T的this地址，用于调用函数的
     */
	Timer & registerTimer(
        otm_timer_id_t id,
        const char * name,
        void * realResponser, void * fun_addr, size_t fun_size,
        uint32_t span_s
#ifdef _MSC_VER
        , void * useResponser
#endif
        );

    void unregisterTimer(void const * processor);
};

template<typename ContextT>
class Manager : public ManagerBase {
public:
    typedef Cpe::Otm::TimerProcessor<ContextT> TimerProcessor;
    typedef void (TimerProcessor::*Fun)(Timer & timer, Memo & memo, uint32_t cur_exec_time_s, ContextT & obj_ctx);

    using ManagerBase::registerTimer;
    template<typename T>
    Timer & registerTimer(
        otm_timer_id_t id,
        const char * name,
        T & r,
        void (T::*fun)(Timer & timer, Memo & memo, uint32_t cur_exec_time_s, ContextT & obj_ctx),
        tl_time_span_t span)
    {
#ifdef _MSC_VER
        Fun save_fun = static_cast<Fun>(fun);
        return this->registerTimer(
            id, name,
            &static_cast<TimerProcessor&>(r), &save_fun, sizeof(save_fun),
            span,
			(void*)&r);
#else
        Fun save_fun = static_cast<Fun>(fun);
        return this->registerTimer(
            id, name,
            &static_cast<TimerProcessor&>(r), &save_fun, sizeof(save_fun),
            span);
#endif
    }

    using ManagerBase::unregisterTimer;
	void unregisterTimer(TimerProcessor const & processor) {
        ManagerBase::unregisterTimer(&processor);
    }


    using ManagerBase::tick;

    template<size_t capacity>
    void tick(uint32_t cur_time_s, ContextT & ctx, MemoBuf<capacity> & buf) {
        tick(cur_time_s, &ctx, buf, capacity);
    }

    static Manager & _cast(otm_manage_t otm) {
        return static_cast<Manager&>(ManagerBase::_cast(otm));
    }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
