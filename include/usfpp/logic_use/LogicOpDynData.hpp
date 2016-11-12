#ifndef USFPP_LOGIC_USE_DYNDATA_H
#define USFPP_LOGIC_USE_DYNDATA_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/System.hpp"
#include "cpepp/dr/Meta.hpp"
#include "usfpp/logic/LogicOpData.hpp"
#include "usf/logic_use/logic_data_dyn.h"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicOpDynData {
public:
    typedef int(*record_cmp_t)(const void *, const void *);

    LogicOpDynData(LogicOpDynData & o) : m_data(o.m_data) {
    }

    LogicOpDynData(logic_data_t data = NULL) : m_data(data) {
    }

    template<typename OT>
    LogicOpDynData(OT & owner, LPDRMETA meta, size_t record_capacity = 1)
        : m_data(
            owner.checkCreateData(
                meta,
                record_capacity ? logic_data_calc_capacity(meta, record_capacity, NULL) : 0))
    {
        validate_data();
    }

    operator logic_data_t () const { return m_data; }

    LogicOpDynData & operator=(LogicOpDynData const & o) { m_data = o.m_data; return *this; }
    LogicOpDynData & operator=(logic_data_t data) { m_data = data; return *this; }

    bool isValid(void) const { return m_data != NULL; }

    Cpe::Dr::Meta const & meta(void) const { return *(Cpe::Dr::Meta const *)logic_data_meta(m_data); }
    void * data(void) { return logic_data_data(m_data); }
    const void * data(void) const { return logic_data_data(m_data); }
    size_t capacity(void) const { return logic_data_capacity(m_data); }

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

    Cpe::Dr::Meta const & recordMeta(void) const { return *(Cpe::Dr::Meta const *)logic_data_record_meta(m_data); }
    size_t recordSize(void) const { return logic_data_record_size(m_data); }

    bool isDynamic(void) const { return logic_data_record_is_dyn(m_data) ? true : false; }

    void setRecordCount(size_t count);
    size_t recordCount(void) const { return logic_data_record_count(m_data); }

    void recordReserve(size_t capacity);
    size_t recordCapacity(void) const { return logic_data_record_capacity(m_data); }

    void * record(size_t i);
    void const * record(size_t i) const;

    void * recordAppend(void);
    void recordRemove(size_t pos);
    void recordRemove(void const * o);
    void recordPop(void);

    void recordSort(record_cmp_t cmp) { logic_data_record_sort(m_data, cmp); }
    void * recordFind(void const * key, record_cmp_t cmp) { return logic_data_record_find(m_data, key, cmp); }
    void const * recordFind(void const * key, record_cmp_t cmp) const { return logic_data_record_find(m_data, key, cmp); }

    template<typename T>
    void copy_same_entries_to(T & data, LPDRMETA src_meta = T::META, int policy = 0) {
        copy_same_entries_to(&data, src_meta, sizeof(data), policy);
    }

    template<typename T>
    T & as(void) { return *(T *)data(); }

    template<typename T>
    T const & as(void) const { return *(T const *)data(); }

    template<typename T>
    T & record(size_t i) { return *(T*)this->record(i); }

    template<typename T>
    T const & record(size_t i) const { return *(const T*)this->record(i); }
    
    template<typename T>
    T & recordAppend(void) { return *(T*)this->recordAppend(); }

    const char * dump(mem_buffer_t buffer) const { return logic_data_dump(m_data, buffer); }

    void destory(void) { logic_data_free(m_data); m_data = NULL; }

private:
    void validate_data(void) const;
    logic_data_t m_data;
};

}}

#endif
