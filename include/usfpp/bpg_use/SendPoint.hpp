#ifndef USFPP_BPG_USE_SENDPOINT_H
#define USFPP_BPG_USE_SENDPOINT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/Data.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/bpg_use/bpg_use_sp.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Bpg {

class SendPoint : public Cpe::Utils::SimulateObject {
public:
    operator bpg_use_sp_t() const { return (bpg_use_sp_t)this; }

    const char * name(void) const { return bpg_use_sp_name(*this); }
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(bpg_use_sp_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(bpg_use_sp_app(*this)); }

    PackageManager const & pkgManager(void) const;

    uint64_t clientId(void) const { return bpg_use_sp_client_id(*this); }
    void setClientId(uint64_t client_id) { bpg_use_sp_set_client_id(*this, client_id); }

    Cpe::Dr::MetaLib const & metaLib(void) const;
    Cpe::Dr::Meta const & meta(const char * metaName) const;

    Cpe::Dr::Data dataBuf(LPDRMETA meta, size_t capacity = 0);
    Cpe::Dr::Data dataBuf(size_t capacity);
    Cpe::Dr::Data dataDynBuf(LPDRMETA meta, size_t record_count);

    template<typename T>
    Cpe::Dr::Data dataBuf(size_t capacity = 0) { return dataBuf(Cpe::Dr::MetaTraits<T>::META, capacity); }

    template<typename T>
    Cpe::Dr::Data dataDynBuf(size_t record_count) { return dataDynBuf(Cpe::Dr::MetaTraits<T>::META, record_count); }

    template<typename T>
    T & dataBufT(size_t capacity = 0) { Cpe::Dr::Data d = dataBuf(Cpe::Dr::MetaTraits<T>::META, capacity); return d.as<T>(); }

    template<typename T>
    T & dataDynBufT(size_t record_count) { Cpe::Dr::Data d = dataBuf(Cpe::Dr::MetaTraits<T>::META, record_count); return d.as<T>(); }

    Usf::Bpg::Package & pkgBuf(size_t capacity);

    void send(Usf::Bpg::Package & pkg);
    void send(Cpe::Dr::Data const & data);
    void send(LPDRMETA meta, void const * data, size_t size = 0);

    template<typename T>
    void send(T const & data) {
        send(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    static SendPoint & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
