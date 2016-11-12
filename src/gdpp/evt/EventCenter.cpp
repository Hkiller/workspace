#include "cpepp/utils/ErrorCollector.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/evt/Event.hpp"
#include "gdpp/evt/EventCenter.hpp"
#include "gdpp/evt/Exceptions.hpp"

namespace Gd { namespace Evt {

Event & EventCenter::createEvent(const char * typeName, ssize_t data_capacity) {
    Cpe::Utils::ErrorCollector em;
    gd_evt_t r = gd_evt_create(*this, typeName, data_capacity, em);
    if (r == NULL) {
        em.checkThrowWithMsg< no_responser_error>();
    }
    return *(Event *)r;
}

Event & EventCenter::createEvent(LPDRMETA data_meta, ssize_t data_capacity) {
    Cpe::Utils::ErrorCollector em;
    gd_evt_t r = gd_evt_create_ex(*this, data_meta, data_capacity, em);
    if (r == NULL) {
        em.checkThrowWithMsg< no_responser_error>();
    }
    return *(Event *)r;
}

Event & EventCenter::createDynEvent(const char * typeName, size_t record_capacity) {
    Cpe::Utils::ErrorCollector em;
    gd_evt_t r = gd_evt_dyn_create(*this, typeName, record_capacity, em);
    if (r == NULL) {
        em.checkThrowWithMsg< no_responser_error>();
    }
    return *(Event *)r;
}

Event & EventCenter::createDynEvent(LPDRMETA data_meta, size_t record_capacity) {
    Cpe::Utils::ErrorCollector em;
    gd_evt_t r = gd_evt_dyn_create_ex(*this, data_meta, record_capacity, em);
    if (r == NULL) {
        em.checkThrowWithMsg< no_responser_error>();
    }
    return *(Event *)r;
}

void EventCenter::sendEvent(const char * target, Event & event) {
    event.setTarget(target);

    int r = gd_evt_send(event, 0, 0, 1);
    if (r != 0) {
        throw ::std::runtime_error("send event fail!");
    }
}

struct EventCenterProcessCtx {
    gd_app_context_t m_app;
    EventProcessFun m_fun;
    EventResponser * m_useResponser;
};

void evt_process_fun(gd_evt_t evt, void * ctx, void * arg) {
    EventCenterProcessCtx * processCtx = (EventCenterProcessCtx *)arg;

    try {
        (processCtx->m_useResponser->*(processCtx->m_fun))(gd_evt_target(evt), Event::_cast(evt));
    }
    APP_CTX_CATCH_EXCEPTION(processCtx->m_app, "process event:"); //TODO: record to app
}

void evt_ctx_free(void * ctx) {
    delete (EventCenterProcessCtx *)ctx;
}

ProcessorID
EventCenter::registerResponser(
        const char * oid,
        EventResponser& realResponser, EventProcessFun fun
#ifdef _MSC_VER
        ,EventResponser& useResponser
#endif
        )
{
    EventCenterProcessCtx * ctx = new EventCenterProcessCtx;
    ctx->m_app = app();
    ctx->m_fun = fun;
#ifdef _MSC_VER
    ctx->m_useResponser = &useResponser;
#else
    ctx->m_useResponser = &realResponser;
#endif

    evt_processor_id_t id;
    if (gd_evt_mgr_regist_responser(
            *this, &id,
            oid, evt_process_fun, &realResponser, ctx, evt_ctx_free) != 0)
    {
        delete ctx;

        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s: EventCenter::realResponser: regist fail!",
            name());
    }

    return id;
}

void EventCenter::unregisterResponser(EventResponser & r) {
    gd_evt_mgr_unregist_responser(*this, &r);
}

EventCenter & EventCenter::_cast(gd_evt_mgr_t mgr) {
    if (mgr == NULL) {
        throw ::std::runtime_error("Gd::Evt::EventCenter::_cast: input mgr is NULL!");
    }

    return *(EventCenter*)mgr;
}

EventCenter & EventCenter::instance(gd_app_context_t app, const char * name) {
    gd_evt_mgr_t mgr = gd_evt_mgr_find_nc(app, name);
    if (mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "gd_evt_mgr %s not exist!", name ? name : "default");
    }

    return *(EventCenter*)mgr;
}

}}


