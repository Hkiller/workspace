#include "gdpp/app/Log.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/mongo_driver/Package.hpp"
#include "usf/mongo_driver/mongo_driver.h"
#include "usfpp/mongo_cli/CliProxy.hpp"

namespace Usf { namespace Mongo {

CliProxy & CliProxy::_cast(mongo_cli_proxy_t agent) {
    if (agent == NULL) {
        throw ::std::runtime_error("Usf::Mongo::CliProxy::_cast: input agent is NULL!");
    }

    return *(CliProxy*)agent;
}

CliProxy & CliProxy::instance(gd_app_context_t app, const char * name) {
    mongo_cli_proxy_t proxy = mongo_cli_proxy_find_nc(app, name);
    if (proxy == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "mongo_agent %s not exist!", name ? name : "default");
    }

    return *(CliProxy*)proxy;
}

bool CliProxy::isEnable(void) const {
    return mongo_driver_is_enable(mongo_cli_proxy_driver(*this)) ? true : false;
}

void CliProxy::send(logic_require_t require, mongo_pkg_t pkg, LPDRMETA result_meta, int result_count_init, const char * prefix,
                    mongo_cli_pkg_parser parser, void * parse_ctx)
{
    if (mongo_cli_proxy_send(*this, pkg, require, result_meta, result_count_init, prefix, parser, parse_ctx) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s: send request fail!", name().c_str());
    }
}

void CliProxy::send(mongo_pkg_t pkg) {
    if (mongo_cli_proxy_send(*this, pkg, NULL, NULL, 0, NULL, NULL, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s: send request fail!", name().c_str());
    }
}

Package & CliProxy::pkgBuf(void) {
    mongo_pkg_t pkg_buf = mongo_cli_proxy_pkg_buf(*this);
    if (pkg_buf == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: pkg-buf is NULL!", name().c_str());
    }

    return Package::_cast(pkg_buf);
}

}}
