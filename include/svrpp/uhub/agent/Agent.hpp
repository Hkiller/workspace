#ifndef SVRPP_UHUB_AGENT_AGENT_H
#define SVRPP_UHUB_AGENT_AGENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "svr/uhub/agent/uhub_agent.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace UHub {

class Agent : public Cpe::Utils::SimulateObject {
public:
    operator uhub_agent_t() const { return (uhub_agent_t)this; }

    const char * name(void) const { return uhub_agent_name(*this); }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)uhub_agent_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)uhub_agent_app(*this); }

    void sendNotifyPkg(
        dp_req_t pkg,
        void const * carry_data = NULL, size_t carry_data_size = 0);

    void sendNotifyData(
        void const * data, uint16_t data_size, LPDRMETA meta,
        void const * carry_data = NULL, size_t carry_data_size = 0);

    template<typename T>
    void sendNotifyData(T const & data) {
        sendNotifyData(&data, Cpe::Dr::MetaTraits<T>::data_size(data), Cpe::Dr::MetaTraits<T>::META);
    }

    template<typename T, typename T2>
    void sendNotifyData( T const & data, T2 const & carry_data) {
        sendNotifyData(
            &data, Cpe::Dr::MetaTraits<T>::data_size(data), Cpe::Dr::MetaTraits<T>::META,
            &carry_data, Cpe::Dr::MetaTraits<T2>::data_size(carry_data));
    }

    static Agent & instance(gd_app_context_t app, const char * name = NULL);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
