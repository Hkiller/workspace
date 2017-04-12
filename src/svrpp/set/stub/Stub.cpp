#include "cpe/dr/dr_metalib_manage.h"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "svrpp/set/stub/Stub.hpp"

namespace Svr { namespace Set {

Stub & Stub::instance(gd_app_context_t app, const char * name) {
    set_svr_stub_t stub = set_svr_stub_find_nc(app, name);
    if (stub == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "set_svr_stub %s not exist!", name);
    }

    return *(Stub*)stub;
}

SvrType const & Stub::svrType(uint16_t svr_type_id) const {
    SvrType const * r = findSvrType(svr_type_id);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "Stub %s: svr type %d not exist!", name(), svr_type_id);
    }

    return *r;
}

SvrType const & Stub::svrType(const char * svr_type_name) const {
    SvrType const * r = findSvrType(svr_type_name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "Stub %s: svr type %s not exist!", name(), svr_type_name);
    }

    return *r;
}

void Stub::sendReqPkg(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    dp_req_t pkg,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_req_pkg(*this, svr_type, svr_id, sn, pkg, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send req pkg to %d.%d fail!",
            name(), svr_type, svr_id);
    }
}

void Stub::sendReqData(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_req_data(*this, svr_type, svr_id, sn, data, data_size, meta, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send %s(size=%d) to %d.%d fail!",
            name(), dr_meta_name(meta), data_size, svr_type, svr_id);
    }
}

void Stub::sendReqCmd(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    uint32_t cmd,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_req_cmd(*this, svr_type, svr_id, sn, cmd, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send cmd %d to %d.%d fail!",
            name(), cmd, svr_type, svr_id);
    }
}

void Stub::sendNotifyPkg(
    uint16_t svr_type, uint16_t svr_id, 
    dp_req_t pkg,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_notify_pkg(*this, svr_type, svr_id, 0, pkg, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send notify pkg to %d.%d fail!",
            name(), svr_type, svr_id);
    }
}

void Stub::sendNotifyData(
    uint16_t svr_type, uint16_t svr_id, 
    void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_notify_data(*this, svr_type, svr_id, 0, data, data_size, meta, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send %s(size=%d) to %d.%d fail!",
            name(), dr_meta_name(meta), data_size, svr_type, svr_id);
    }
}

void Stub::sendNotifyCmd(
    uint16_t svr_type, uint16_t svr_id,
    uint32_t cmd,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_notify_cmd(*this, svr_type, svr_id, 0, cmd, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send cmd %d to %d.%d fail!",
            name(), cmd, svr_type, svr_id);
    }
}

void Stub::sendResponsePkg(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    dp_req_t pkg,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_response_pkg(*this, svr_type, svr_id, sn, pkg, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send response pkg to %d.%d fail!",
            name(), svr_type, svr_id);
    }
}

void Stub::sendResponseData(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_response_data(*this, svr_type, svr_id, sn, data, data_size, meta, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send %s(size=%d) to %d.%d fail!",
            name(), dr_meta_name(meta), data_size, svr_type, svr_id);
    }
}

void Stub::sendResponseCmd(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    uint32_t cmd,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_response_cmd(*this, svr_type, svr_id, sn, cmd, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send cmd %d to %d.%d fail!",
            name(), cmd, svr_type, svr_id);
    }
}

void Stub::replyPkg(dp_req_t req, dp_req_t pkg) {
    if (set_svr_stub_reply_pkg(*this, req, pkg) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "Stub %s: reply pkg fail!", name());
    }
}

void Stub::replyData(dp_req_t req, void const * data, uint16_t data_size, LPDRMETA meta) {
    if (set_svr_stub_reply_data(*this, req, data, data_size, meta) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: reply %s(%d) fail!",
            name(), dr_meta_name(meta), data_size);
    }
}

void Stub::replyCmd(dp_req_t req, uint32_t cmd) {
    if (set_svr_stub_reply_cmd(*this, req, cmd) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "Stub %s: reply %d fail!", name(), cmd);
    }
}

PkgBody & Stub::outgoingBuf(size_t capacity) {
    dp_req_t pkg = set_svr_stub_outgoing_pkg_buf(*this, capacity);

    if (pkg == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: get outgoing buf fail, capacity=%d!",
            name(), (int)capacity);
    }

    return *(PkgBody*)pkg;
}

void * Stub::pkgToData(dp_req_t pkg_body, uint16_t svr_type_id, LPDRMETA data_meta, size_t * data_capacity) {
    void * data = set_svr_stub_pkg_to_data(*this, pkg_body, svr_type_id, data_meta, data_capacity);

    if (data == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: pkg to data fail, svr_type_id=%d, meta=%s!",
            name(), svr_type_id, dr_meta_name(data_meta));
    }

    return data;
}

}}
