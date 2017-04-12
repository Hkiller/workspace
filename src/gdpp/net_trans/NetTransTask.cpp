#include "cpepp/utils/ErrorCollector.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/net_trans/NetTransMgr.hpp"

namespace Gd { namespace NetTrans {

void NetTransTask::set_post_to(const char * uri, const char * data, size_t data_len) {
    if (net_trans_task_set_post_to(*this, uri, data, data_len) != 0) {
        APP_CTX_THROW_EXCEPTION(
            net_trans_manage_app(net_trans_task_manage(*this)),
            ::std::runtime_error,
            "net trans task set post to %s fail!", uri);
    }
}
    
void NetTransTask::set_get(const char * uri) {
    if (net_trans_task_set_get(*this, uri) != 0) {
        APP_CTX_THROW_EXCEPTION(
            net_trans_manage_app(net_trans_task_manage(*this)),
            ::std::runtime_error,
            "net trans task set get %s fail!", uri);
    }
}
    
void NetTransTask::start(void) {
    if (net_trans_task_start(*this) != 0) {
        APP_CTX_THROW_EXCEPTION(
            net_trans_manage_app(net_trans_task_manage(*this)),
            ::std::runtime_error,
            "net trans task start fail!");
    }
}

void NetTransTask::set_skip_data(ssize_t skip_size) {
    if (net_trans_task_set_skip_data(*this, skip_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            net_trans_manage_app(net_trans_task_manage(*this)),
            ::std::runtime_error,
            "net trans task set skip data fail!");
    }
}

void NetTransTask::set_timeout(uint64_t timeout_ms) {
    if (net_trans_task_set_timeout(*this, timeout_ms) != 0) {
        APP_CTX_THROW_EXCEPTION(
            net_trans_manage_app(net_trans_task_manage(*this)),
            ::std::runtime_error,
            "net trans task set timeout fail!");
    }
}

const char * NetTransTask::buffer_to_string(void) {
    const char * r = net_trans_task_buffer_to_string(*this);

    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            net_trans_manage_app(net_trans_task_manage(*this)),
            ::std::runtime_error,
            "net trans task get buffer string error!");
    }

    return r;
}

struct NetTransTaskProcessCtx {
    gd_app_context_t m_app;
    NetTransProcessFun m_fun;
    NetTransProcessor * m_useResponser;
};

void net_trans_task_process_fun(net_trans_task_t task, void * ctx) {
    NetTransTaskProcessCtx * processCtx = (NetTransTaskProcessCtx *)ctx;

    try {
        (processCtx->m_useResponser->*(processCtx->m_fun))(*(NetTransTask*)task);
    }
    APP_CTX_CATCH_EXCEPTION(processCtx->m_app, "process net_trans_task:");
}

void net_trans_task_process_ctx_free(void * ctx) {
    delete (NetTransTaskProcessCtx *)ctx;
}

void NetTransTask::set_commit_to(
    NetTransProcessor& realResponser, NetTransProcessFun fun
#ifdef _MSC_VER
        , NetTransProcessor& useResponser
#endif
        )
{
    NetTransTaskProcessCtx * ctx = new NetTransTaskProcessCtx;
    ctx->m_app = net_trans_manage_app(net_trans_task_manage(*this));
    ctx->m_fun = fun;
#ifdef _MSC_VER
    ctx->m_useResponser = &useResponser;
#else
    ctx->m_useResponser = &realResponser;
#endif

    net_trans_task_set_commit_op(*this, net_trans_task_process_fun, ctx, net_trans_task_process_ctx_free);
}

struct NetTransTaskProgressCtx {
    gd_app_context_t m_app;
    NetTransProgressFun m_fun;
    NetTransProcessor * m_useResponser;
};

void net_trans_task_progress_fun(net_trans_task_t task, void * ctx, double dltotal, double dlnow) {
    NetTransTaskProgressCtx * progressCtx = (NetTransTaskProgressCtx *)ctx;

    try {
        (progressCtx->m_useResponser->*(progressCtx->m_fun))(*(NetTransTask*)task, dltotal, dlnow);
    }
    APP_CTX_CATCH_EXCEPTION(progressCtx->m_app, "progress net_trans_task:");
}

void net_trans_task_progress_ctx_free(void * ctx) {
    delete (NetTransTaskProgressCtx *)ctx;
}

void NetTransTask::set_on_progress(
    NetTransProcessor& realResponser, NetTransProgressFun fun
#ifdef _MSC_VER
        , NetTransProcessor& useResponser
#endif
        )
{
    NetTransTaskProgressCtx * ctx = new NetTransTaskProgressCtx;
    ctx->m_app = net_trans_manage_app(net_trans_task_manage(*this));
    ctx->m_fun = fun;
#ifdef _MSC_VER
    ctx->m_useResponser = &useResponser;
#else
    ctx->m_useResponser = &realResponser;
#endif

    net_trans_task_set_progress_op(*this, net_trans_task_progress_fun, ctx, net_trans_task_progress_ctx_free);
}

struct NetTransTaskWriteCtx {
    gd_app_context_t m_app;
    NetTransWriteFun m_fun;
    NetTransProcessor * m_useResponser;
};

void net_trans_task_write_fun(net_trans_task_t task, void * ctx, void * data, size_t data_size) {
    NetTransTaskWriteCtx * writeCtx = (NetTransTaskWriteCtx *)ctx;

    try {
        (writeCtx->m_useResponser->*(writeCtx->m_fun))(*(NetTransTask*)task, data, data_size);
    }
    APP_CTX_CATCH_EXCEPTION(writeCtx->m_app, "write net_trans_task:");
}

void net_trans_task_write_ctx_free(void * ctx) {
    delete (NetTransTaskWriteCtx *)ctx;
}

void NetTransTask::set_on_write(
    NetTransProcessor& realResponser, NetTransWriteFun fun
#ifdef _MSC_VER
        , NetTransProcessor& useResponser
#endif
        )
{
    NetTransTaskWriteCtx * ctx = new NetTransTaskWriteCtx;
    ctx->m_app = net_trans_manage_app(net_trans_task_manage(*this));
    ctx->m_fun = fun;
#ifdef _MSC_VER
    ctx->m_useResponser = &useResponser;
#else
    ctx->m_useResponser = &realResponser;
#endif

    net_trans_task_set_write_op(*this, net_trans_task_write_fun, ctx, net_trans_task_write_ctx_free);
}

void NetTransTask::check_capacity(LPDRMETA meta) const {
    
}

}}

