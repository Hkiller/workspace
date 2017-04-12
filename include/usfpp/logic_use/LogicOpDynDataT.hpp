#ifndef USFPP_LOGIC_USE_DYNDATA_T_H
#define USFPP_LOGIC_USE_DYNDATA_T_H
#include "LogicOpDynData.hpp"

namespace Usf { namespace Logic {

template<typename DataT>
class LogicOpDynDataT : public LogicOpDynData {
public:
    typedef typename Cpe::Dr::MetaTraits<DataT>::dyn_element_type RecordType;
    typedef DataT DataType;

    LogicOpDynDataT(LogicOpDynDataT & o) : LogicOpDynData(o) {
    }

    LogicOpDynDataT(logic_data_t data = NULL) : LogicOpDynData(data) {
    }

    template<typename OT>
    LogicOpDynDataT(OT & owner, size_t record_capacity = 1)
        : LogicOpDynData(owner, Cpe::Dr::MetaTraits<DataT>::META, record_capacity) 
    {
    }

    using LogicOpDynData::recordAppend;
    using LogicOpDynData::record;
    using LogicOpDynData::as;
    using LogicOpDynData::recordSort;
    using LogicOpDynData::recordFind;

    DataType & as(void) { return LogicOpDynData::as<DataType>(); }
    DataType const & as(void) const { return LogicOpDynData::as<DataType>(); }

    RecordType & recordAppend(void) { return LogicOpDynData::recordAppend<RecordType>(); }

    RecordType & recordAppend(RecordType const & o) { 
        RecordType & r = recordAppend();
        memcpy(&r, &o, sizeof(o));
        return r;
    }

    RecordType & record(size_t i) { return *(RecordType*)LogicOpDynData::record(i); }
    RecordType const & record(size_t i) const { return *(RecordType const *)LogicOpDynData::record(i); }

    void recordSort(int (*cmp)(RecordType const * l, RecordType const * r)) { LogicOpDynData::recordSort((record_cmp_t)cmp); }

    RecordType * recordFind(RecordType const & key, record_cmp_t cmp) { return (DataT *)LogicOpDynData::recordFind(&key, cmp); }
    RecordType const * recordFind(RecordType const & key, record_cmp_t cmp) const { return (DataT const *)LogicOpDynData::recordFind(&key, cmp); }

    RecordType * recordFind(RecordType const & key, int (*cmp)(RecordType const * l, RecordType const * r)) { return (RecordType *)LogicOpDynData::recordFind(&key, (record_cmp_t)cmp); }
    RecordType const * recordFind(RecordType const & key, int (*cmp)(RecordType const * l, RecordType const * r)) const { return (RecordType *)LogicOpDynData::recordFind(&key, (record_cmp_t)cmp); }

    void recordRemove(size_t pos) { return LogicOpDynData::recordRemove(pos); }
    void recordRemove(RecordType const * o) { return LogicOpDynData::recordRemove((void const*)o); }
};

}}

#endif
