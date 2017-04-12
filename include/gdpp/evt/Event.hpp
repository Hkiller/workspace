#ifndef GDPP_EVT_EVENT_H
#define GDPP_EVT_EVENT_H
#include "cpe/utils/stream.h"
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/Data.hpp"
#include "gd/evt/evt_read.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Gd { namespace Evt {

class Event : public Cpe::Utils::SimulateObject {
public:
    operator gd_evt_t (void) const { return (gd_evt_t)this; }

    Cpe::Utils::CString const & type(void) const { return Cpe::Utils::CString::_cast(gd_evt_type(*this)); }

    Cpe::Utils::CString const & target(void) const { return Cpe::Utils::CString::_cast(gd_evt_target(*this)); }
    cpe_hash_string_t targetHs(void) const { return gd_evt_target_hs(*this); }
    void setTarget(const char * target);

    /*carry operations*/
    Cpe::Dr::Meta const * carryDataMeta(void) const { return (Cpe::Dr::Meta const *)gd_evt_carry_meta(*this); }
    size_t carryDataCapacity(void) const { return gd_evt_carry_data_capacity(*this); }
    void * carryData(void) { return gd_evt_carry_data(*this); }
    void const * carryData(void) const { return gd_evt_carry_data(*this); }

    void * data(void) { return gd_evt_data(*this); }
    const void * data(void) const { return gd_evt_data(*this); }
    size_t capacity(void) const { return gd_evt_data_capacity(*this); }

    /*data operations*/
    Cpe::Dr::Meta const & meta(void) const { return Cpe::Dr::Meta::_cast(gd_evt_meta(*this)); }

    Cpe::Dr::ConstData args(void) const { return Cpe::Dr::ConstData(data(), meta(), capacity()); }
    Cpe::Dr::Data args(void) {return Cpe::Dr::Data(data(), meta(), capacity()); }

    Cpe::Dr::ConstDataElement operator[] (const char * name) const { return args()[name]; }
    Cpe::Dr::DataElement operator[] (const char * name) { return args()[name]; }

    bool is_valid(const char * name) const {
        return args().is_valid(name);
    }

    const char * dump(mem_buffer_t buffer) const;
    void dump(write_stream_t stream) const { gd_evt_dump(stream, *this); }

    template<typename T>
    T & as(void) { return *(T*)data(); }

    template<typename T>
    T const & as(void) const { return *(T const *)data(); }

    template<typename T>
    T * check_as(void) { return Cpe::Dr::MetaTraits<T>::META == meta() ? (T *)data() : NULL; }

    template<typename T>
    T const * check_as(void) const { return Cpe::Dr::MetaTraits<T>::META == meta() ? (T const *)data() : NULL; }

    Event * clone(mem_allocrator_t alloc = NULL) const;

    void destory(void);

    static Event & _cast(gd_evt_t evt);
    static Event & _cast(tl_event_t tl_evt);

};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
