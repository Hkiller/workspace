#ifndef USFPP_MONGO_CLI_PROXY_H
#define USFPP_MONGO_CLI_PROXY_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/System.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Mongo {

class CliProxy : public Cpe::Utils::SimulateObject {
public:
    operator mongo_cli_proxy_t() const { return (mongo_cli_proxy_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(mongo_cli_proxy_name(*this)); }
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(mongo_cli_proxy_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(mongo_cli_proxy_app(*this)); }

    bool isEnable(void) const;
    Package & pkgBuf(void);

    void send(
        logic_require_t require, mongo_pkg_t pkg,
        LPDRMETA result_meta = NULL, int result_count_init = 0, const char * prefix = NULL,
        mongo_cli_pkg_parser parser = NULL, void * parse_ctx = 0);

    void send(mongo_pkg_t pkg);

    template<typename T>
    void query(
        logic_require_t require, mongo_pkg_t pkg, int result_count_init = 0, const char * prefix = NULL,
        mongo_cli_pkg_parser parser = NULL, void * parse_ctx = 0)
    {
        send(require, pkg, Cpe::Dr::MetaTraits<T>::META, result_count_init, prefix, parser, parse_ctx);
    }

    static CliProxy & _cast(mongo_cli_proxy_t agent);
    static CliProxy & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
