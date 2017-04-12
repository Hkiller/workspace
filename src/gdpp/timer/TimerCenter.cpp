#include <limits>
#include "cpepp/utils/ErrorCollector.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/timer/TimerCenter.hpp"

namespace Gd { namespace Timer {

struct TimerCenterProcessCtx {
    gd_app_context_t m_app;
    TimerProcessFun m_fun;
    TimerProcessor * m_useResponser;
};

void timer_process_fun(void * ctx, gd_timer_id_t timer_id, void * arg) {
    TimerCenterProcessCtx * processCtx = (TimerCenterProcessCtx *)arg;

    try {
        (processCtx->m_useResponser->*(processCtx->m_fun))(timer_id);
    }
    APP_CTX_CATCH_EXCEPTION(processCtx->m_app, "process timer:");
}

void timer_ctx_free(void * ctx) {
    delete (TimerCenterProcessCtx *)ctx;
}

TimerID
TimerCenter::registerTimer(
        TimerProcessor& realResponser, TimerProcessFun fun
        , tl_time_span_t delay, tl_time_span_t span, int repeatCount
#ifdef _MSC_VER
        ,TimerProcessor& useResponser
#endif
        )
{
    TimerCenterProcessCtx * ctx = new TimerCenterProcessCtx;
    ctx->m_app = app();
    ctx->m_fun = fun;
#ifdef _MSC_VER
    ctx->m_useResponser = &useResponser;
#else
    ctx->m_useResponser = &realResponser;
#endif

    gd_timer_id_t id;
    if (gd_timer_mgr_regist_timer(
            *this, &id,
            timer_process_fun, &realResponser, ctx, timer_ctx_free,
            delay, span, repeatCount) != 0)
    {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s: TimerCenter::realResponser: regist fail!",
            name());
    }

    return id;
}

TimerCenter & TimerCenter::_cast(gd_timer_mgr_t mgr) {
    if (mgr == NULL) {
        throw ::std::runtime_error("Gd::Evt::TimerCenter::_cast: input mgr is NULL!");
    }

    return *(TimerCenter*)mgr;
}

TimerCenter & TimerCenter::instance(gd_app_context_t app, const char * name) {
    gd_timer_mgr_t mgr = gd_timer_mgr_find_nc(app, name);
    if (mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "gd_timer_mgr %s not exist!", name ? name : "default");
    }

    return *(TimerCenter*)mgr;
}

}}


