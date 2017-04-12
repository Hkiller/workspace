#ifndef SVRPP_SET_LOGIC_SENDPOINT_H
#define SVRPP_SET_LOGIC_SENDPOINT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "cpepp/dp/System.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "svr/set/logic/set_logic_sp.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Set {

class SendPoint : public Cpe::Utils::SimulateObject {
public:
    operator set_logic_sp_t() const { return (set_logic_sp_t)this; }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)set_logic_sp_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)set_logic_sp_app(*this); }

    Stub & stub(void) { return *(Stub*)set_logic_sp_stub(*this); }
    Stub const & stub(void) const { return *(Stub const *)set_logic_sp_stub(*this); }

    const char * name(void) const { return set_logic_sp_name(*this); }

    uint16_t responseFromSvrType(logic_require_t require) const;
    uint16_t responseFromSvrId(logic_require_t require) const;

    PkgBody & outgoingBuf(size_t capacity);

    void * pkgToData(dp_req_t pkg_body, uint16_t svr_type_id, LPDRMETA data_meta, size_t * data_capacity = NULL);

    template<typename T>
    T & pkgToData(dp_req_t pkg_body, uint16_t svr_type_id = 0, size_t * data_capacity = NULL) {
        return *(T*)pkgToData(pkg_body, svr_type_id, Cpe::Dr::MetaTraits<T>::META, data_capacity);
    }

    void sendData(
        uint16_t to_svr_type, uint16_t to_svr_id,
        LPDRMETA meta, void const * data, size_t size,
        void const * carry_data = NULL, size_t carry_data_size = 0,
        logic_require_t require = NULL);

    void sendCmd(
        uint16_t to_svr_type, uint16_t to_svr_id,
        uint32_t cmd,
        void const * carry_data = NULL, size_t carry_data_size = 0,
        logic_require_t require = NULL);

    void sendPkg(
        uint16_t to_svr_type, uint16_t to_svr_id,
        dp_req_t pkg,
        void const * carry_data, size_t carry_data_size,
        logic_require_t require = NULL);

    void sendPkg(
        uint16_t to_svr_type, uint16_t to_svr_id,
        dp_req_t pkg,
        logic_require_t require = NULL)
    {
        sendPkg(to_svr_type, to_svr_id, pkg, NULL, 0, require);
    }

    void sendPkg(dp_req_t pkg, logic_require_t require = NULL);

    template<typename T>
    void sendData(uint16_t to_svr_type, uint16_t to_svr_id, T const & data, logic_require_t require = NULL) {
        sendData(
            to_svr_type, to_svr_id,
            Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data),
            NULL, 0,
            require);
    }

    template<typename T>
    void sendData(uint16_t to_svr_type, uint16_t to_svr_id, T const & data, Usf::Logic::LogicOpRequire & require) {
        sendData(
            to_svr_type, to_svr_id,
            Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data),
            NULL, 0,
            require);
    }

    template<typename T, typename T2>
    void sendData(uint16_t to_svr_type, uint16_t to_svr_id, T const & data, T2 const & carry, logic_require_t require = NULL) {
        sendData(
            to_svr_type, to_svr_id,
            Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data),
            &carry, Cpe::Dr::MetaTraits<T2>::data_size(carry),
            require);
    }

    template<typename T>
    void sendCmd(uint16_t to_svr_type, uint16_t to_svr_id, uint32_t cmd, T const & carry, logic_require_t require = NULL) {
        sendCmd(
            to_svr_type, to_svr_id,
            cmd,
            &carry, Cpe::Dr::MetaTraits<T>::data_size(carry),
            require);
    }

    void sendCmd(uint16_t to_svr_type, uint16_t to_svr_id, uint32_t cmd, logic_require_t require) {
        sendCmd(to_svr_type, to_svr_id, cmd, NULL, 0, require);
    }

    static SendPoint & instance(gd_app_context_t app, const char * name = NULL);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
