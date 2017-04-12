#include <stdexcept>
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "svrpp/conn/net_cli/NetClient.hpp" 

namespace Svr { namespace Conn {

NetClient & NetClient::instance(gd_app_context_t app, const char * name) {
    conn_net_cli_t cli = conn_net_cli_find_nc(app, name);
    if (cli == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "conn_net_cli %s not exist!", name);
    }

    return *(NetClient*)cli;
}

void NetClient::setSvr(const char * ip, uint16_t port) {
    if (conn_net_cli_set_svr(*this, ip, port) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s set svr to %s:%d fail!", name(), ip, port);
    }
}

void NetClient::sendPkg(uint16_t to_svr, uint32_t sn, LPDRMETA meta, void const * data, uint16_t data_len) {
    if (conn_net_cli_send(*this, to_svr, sn, meta, data, data_len) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s send pkg %s to svr %d fail!", name(), dr_meta_name(meta), to_svr);
    }
}

void NetClient::sendData(uint16_t to_svr, uint32_t sn, LPDRMETA meta, void const * data, uint16_t data_len) {
    if (conn_net_cli_send_data(*this, to_svr, sn, meta, data, data_len) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s send data %s to svr %d fail!", name(), dr_meta_name(meta), to_svr);
    }
}

void NetClient::sendCmd(uint16_t to_svr, uint32_t sn, uint32_t cmd) {
    if (conn_net_cli_send_cmd(*this, to_svr, sn, cmd) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s send cmd %d to svr %d fail!", name(), cmd, to_svr);
    }
}

}}
