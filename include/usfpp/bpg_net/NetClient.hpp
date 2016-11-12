#ifndef USFPP_BPG_NET_NETCLIENT_H
#define USFPP_BPG_NET_NETCLIENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/bpg_net/bpg_net_client.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Bpg {

class NetClient : public Cpe::Utils::SimulateObject {
public:
    operator bpg_net_client_t() const { return (bpg_net_client_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(bpg_net_client_name(*this)); }
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(bpg_net_client_app(*this)); }

    PackageManager & pkgManager(void);

    static NetClient & _cast(bpg_net_client_t net_client);
    static NetClient & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
