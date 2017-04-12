#ifndef USFPP_BPG_CLI_NETCLIENT_H
#define USFPP_BPG_CLI_NETCLIENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/Data.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/bpg_cli/bpg_cli_proxy.h"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Bpg {

class CliProxy : public Cpe::Utils::SimulateObject {
public:
    operator bpg_cli_proxy_t() const { return (bpg_cli_proxy_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(bpg_cli_proxy_name(*this)); }
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(bpg_cli_proxy_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(bpg_cli_proxy_app(*this)); }

    PackageManager & pkgManager(void);

    uint64_t clientId(void) const { return bpg_cli_proxy_client_id(*this); }
    void setClientId(uint64_t id) { bpg_cli_proxy_set_client_id(*this, id); }

    Cpe::Dr::MetaLib const & metaLib(void) const;
    Cpe::Dr::Meta const & meta(const char * metaName) const;

    size_t bufCapacity(void) const { return bpg_cli_proxy_buf_capacity(*this); }
    Cpe::Dr::Data dataBuf(LPDRMETA meta, size_t capacity = 0);
    Cpe::Dr::Data dataBuf(void);
    Usf::Bpg::Package & pkgBuf(void);

    void send(logic_require_t requrest, Usf::Bpg::Package & pkg);
    void send(logic_require_t requrest, Cpe::Dr::Data const & data);
    void send(logic_require_t requrest, LPDRMETA meta, void const * data, size_t size);
    void send(logic_require_t requrest, uint32_t cmd);

    template<typename T>
    void send(logic_require_t requrest, T const & data) {
        send(requrest, Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    void send(Usf::Bpg::Package & pkg);
    void send(Cpe::Dr::Data const & data);
    void send(LPDRMETA meta, void const * data, size_t size);
    void send(uint32_t cmd);

    template<typename T>
    void send(T const & data) {
        send(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    template<typename T>
    Cpe::Dr::Data dataBuf(size_t capacity = 0) {
        return dataBuf(Cpe::Dr::MetaTraits<T>::META, capacity);
    }

    static CliProxy & _cast(bpg_cli_proxy_t cli_proxy);
    static CliProxy & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
