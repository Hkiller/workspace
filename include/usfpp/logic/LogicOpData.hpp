#ifndef USFPP_LOGIC_DATA_H
#define USFPP_LOGIC_DATA_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/dr/dr_data.h"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/Data.hpp"
#include "usf/logic/logic_data.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Logic {

class LogicOpData : public Cpe::Utils::SimulateObject {
public:
    operator logic_data_t () const { return (logic_data_t)this; }

    Cpe::Dr::Meta const & meta(void) const { return *(Cpe::Dr::Meta const *)logic_data_meta(*this); }
    void * data(void) { return logic_data_data(*this); }
    const void * data(void) const { return logic_data_data(*this); }
    size_t capacity(void) const { return logic_data_capacity(*this); }

    Cpe::Dr::Data asDrData(void) { return Cpe::Dr::Data(data(), meta(), capacity()); }
    Cpe::Dr::ConstData asDrData(void) const { return Cpe::Dr::ConstData(data(), meta(), capacity()); }

    void copy_same_entries_from(const void * src, LPDRMETA src_meta, size_t srcCapacity = 0, int policy = 0) {
        dr_meta_copy_same_entry(
            data(), capacity(), meta(),
            src, srcCapacity, src_meta,
            policy, 0);
    }

    template<typename T>
    void copy_same_entries_from(T const & data, LPDRMETA src_meta = T::META, int policy = 0) {
        copy_same_entries_from(&data, src_meta, sizeof(data), policy);
    }

    void copy_same_entries_to(void * desc, LPDRMETA desc_meta, size_t descCapacity = 0, int policy = 0) const {
        dr_meta_copy_same_entry(
            desc, descCapacity, desc_meta,
            data(), capacity(), meta(),
            policy, 0);
    }

    template<typename T>
    void copy_same_entries_to(T & data, LPDRMETA src_meta = T::META, int policy = 0) {
        copy_same_entries_to(&data, src_meta, sizeof(data), policy);
    }

    template<typename T>
    T & as(void) { return *(T *)data(); }

    template<typename T>
    T const & as(void) const { return *(T const *)data(); }

    const char * dump(mem_buffer_t buffer) const { return logic_data_dump(*this, buffer); }

    void destory(void) { logic_data_free(*this); }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
