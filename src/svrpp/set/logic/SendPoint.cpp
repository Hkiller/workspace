#include <stdexcept>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpepp/dp/Request.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "svrpp/set/logic/SendPoint.hpp" 
#include "svrpp/set/stub/Stub.hpp" 

namespace Svr { namespace Set {

void SendPoint::sendData(
    uint16_t to_svr_type, uint16_t to_svr_id,
    LPDRMETA meta, void const * data, size_t size,
    void const * carry_data, size_t carry_data_size,
    logic_require_t require)
{
    if (set_logic_sp_send_req_data(*this, to_svr_type, to_svr_id, meta, data, size, carry_data, carry_data_size, require) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: send data %s(len=%d) fail!", name(), dr_meta_name(meta), (int)size);
    }
}

void SendPoint::sendCmd(
    uint16_t to_svr_type, uint16_t to_svr_id,
    uint32_t cmd,
    void const * carry_data, size_t carry_data_size,
    logic_require_t require)
{
    if (set_logic_sp_send_req_cmd(*this, to_svr_type, to_svr_id, cmd, carry_data, carry_data_size, require) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: send cmd %d fail!", name(), (int)cmd);
    }
}

void SendPoint::sendPkg(
    uint16_t to_svr_type, uint16_t to_svr_id,
    dp_req_t pkg,
    void const * carry_data, size_t carry_data_size,
    logic_require_t require)
{
    if (set_logic_sp_send_req_pkg(*this, to_svr_type, to_svr_id, pkg, carry_data, carry_data_size, require) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: send pkg fail!", name());
    }
}

void SendPoint::sendPkg(dp_req_t pkg, logic_require_t require) {
    if (set_logic_sp_send_pkg(*this, pkg, require) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: send pkg fail!", name());
    }
}

uint16_t SendPoint::responseFromSvrType(logic_require_t require) const {
    uint16_t svr_type;
    if (set_logic_sp_response_from_svr_type(require, &svr_type) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s get responseFromSvrType fail!", name());
    }

    return svr_type;
}

uint16_t SendPoint::responseFromSvrId(logic_require_t require) const {
    uint16_t svr_id;
    if (set_logic_sp_response_from_svr_id(require, &svr_id) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s get responseFromSvrType fail!", name());
    }

    return svr_id;
}

SendPoint & SendPoint::instance(gd_app_context_t app, const char * name) {
    set_logic_sp_t sp = set_logic_sp_find_nc(app, name);
    if (sp == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "set_logic_sp %s not exist!", name);
    }

    return *(SendPoint*)sp;
}

PkgBody & SendPoint::outgoingBuf(size_t capacity) {
    return stub().outgoingBuf(capacity);
}

void * SendPoint::pkgToData(dp_req_t pkg_body, uint16_t svr_type_id, LPDRMETA data_meta, size_t * data_capacity) {
    return stub().pkgToData(pkg_body, svr_type_id, data_meta, data_capacity);
}

}}
