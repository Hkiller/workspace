#ifndef SVRPP_CONN_LOGIC_SENDPOINT_H
#define SVRPP_CONN_LOGIC_SENDPOINT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "cpepp/dp/System.hpp"
#include "svr/conn/net_logic/conn_net_logic_sp.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Conn {

class NetSendPoint : public Cpe::Utils::SimulateObject {
public:
    operator conn_net_logic_sp_t() const { return (conn_net_logic_sp_t)this; }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)conn_net_logic_sp_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)conn_net_logic_sp_app(*this); }

    const char * name(void) const { return conn_net_logic_sp_name(*this); }

    Cpe::Dp::Request & outgoingPkgBuf(size_t size);

    void sendReq(
        LPDRMETA meta, void const * data, size_t size,
        logic_require_t require = NULL);

    template<typename T>
    void sendReq(T const & data, logic_require_t require = NULL) {
        sendReq(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data), require);
    }

    static NetSendPoint & instance(gd_app_context_t app, const char * name = NULL);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
