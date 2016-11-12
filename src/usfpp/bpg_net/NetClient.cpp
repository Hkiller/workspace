#include "gdpp/app/Log.hpp"
#include "usfpp/bpg_net/NetClient.hpp"

namespace Usf { namespace Bpg {

PackageManager &
NetClient::pkgManager(void) {
    bpg_pkg_manage_t pkg_manage = bpg_net_client_pkg_manage(*this);
    if (pkg_manage == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "NetClient %s have no pkg_manage!", name().c_str());
    }

    return *(PackageManager*)pkg_manage;
}

NetClient & NetClient::_cast(bpg_net_client_t net_client) {
    if (net_client == NULL) {
        throw ::std::runtime_error("Usf::Bpg::NetClient::_cast: input net_client is NULL!");
    }

    return *(NetClient*)net_client;
}

NetClient & NetClient::instance(gd_app_context_t app, const char * name) {
    bpg_net_client_t net_client = bpg_net_client_find_nc(app, name);
    if (net_client == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "bpg_net_client %s not exist!", name ? name : "default");
    }

    return *(NetClient*)net_client;
}

}}
