#include <stdexcept>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpepp/dp/Request.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "svrpp/conn/net_logic/NetSendPoint.hpp" 

namespace Svr { namespace Conn {

Cpe::Dp::Request & NetSendPoint::outgoingPkgBuf(size_t size) {
    dp_req_t req = conn_net_logic_sp_outgoing_buf(*this, size);

    if (req == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "conn_net_logic_sp %s: get outgoing pkg buf fail, size=%d", name(), (int)size);
    }

    return *(Cpe::Dp::Request *)req;
}

void NetSendPoint::sendReq(
    LPDRMETA meta, void const * data, size_t size,
    logic_require_t require)
{
    if (conn_net_logic_sp_send_request(*this, meta, data, size, require) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "conn_net_logic_sp %s: send %s(len=%d) fail!",
            name(), dr_meta_name(meta), (int)size);
    }
}

NetSendPoint & NetSendPoint::instance(gd_app_context_t app, const char * name) {
    conn_net_logic_sp_t sp = conn_net_logic_sp_find_nc(app, name);
    if (sp == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "conn_net_logic_sp %s not exist!", name);
    }

    return *(NetSendPoint*)sp;
}

}}
