#ifndef GDPP_TIMER_EVENTCENTER_H
#define GDPP_TIMER_EVENTCENTER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gdpp/app/Application.hpp"
#include "gd/timer/timer_manage.h"
#include "System.hpp"
#include "TimerProcessor.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
# pragma warning(disable:4407)
#endif

namespace Gd { namespace Timer {

class TimerCenter : public Cpe::Utils::SimulateObject {
public:
    operator gd_timer_mgr_t (void) const { return (gd_timer_mgr_t)this; }

    const char * name(void) const { return gd_timer_mgr_name(*this); }
    cpe_hash_string_t name_hs(void) const { return gd_timer_mgr_name_hs(*this); }

    App::Application & app(void) { return App::Application::_cast(gd_timer_mgr_app(*this)); }
    App::Application const & app(void) const { return App::Application::_cast(gd_timer_mgr_app(*this)); }

    tl_t tl(void) const { return  gd_timer_mgr_tl(*this); }

    template<typename T>
    TimerID registerTimer(
        T & r,
        void (T::*fun)(TimerID timerId),
        tl_time_span_t delay,
        tl_time_span_t span,
        int repeatCount = -1)
    {
#ifdef _MSC_VER
        return this->registerTimer(
            r, static_cast<TimerProcessFun>(fun)
            , delay, span, repeatCount
            , *((TimerProcessor*)((void*)&r)));
#else
        return this->registerTimer(
            static_cast<TimerProcessor&>(r), static_cast<TimerProcessFun>(fun),
            delay, span, repeatCount);
#endif
    }

    /*VC编译器处理成员函数地址时有错误，没有生成垫片函数，所以为了正确调用函数指针，必须直接把传入T类型的对象绑定在调用的对象上
      所以传入的realResponser为真实的Responser地址，而useResponser是T的this地址，用于调用函数的
     */
	TimerID registerTimer(
        TimerProcessor& realResponser, TimerProcessFun fun
        , tl_time_span_t delay, tl_time_span_t span, int repeatCount
#ifdef _MSC_VER
        , TimerProcessor& useResponser
#endif
        );

	void unregisterTimer(TimerProcessor & r) { gd_timer_mgr_unregist_timer_by_ctx(*this, &r); }
	void unregisterTimer(TimerID timerId) { gd_timer_mgr_unregist_timer_by_id(*this, timerId); }

    bool haveTimer(TimerID timerId) const { return gd_timer_mgr_have_timer(*this, timerId) ? true : false; }

    static TimerCenter & instance(gd_app_context_t app, const char * name = NULL);
    static TimerCenter & _cast(gd_timer_mgr_t evm);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
