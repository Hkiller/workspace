#ifndef CPEPP_DR_DATA_H
#define CPEPP_DR_DATA_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4521)
#endif

namespace Cpe { namespace Dr {

class ConstDataElement {
public:
    ConstDataElement(const void * data, LPDRMETAENTRY entry, size_t capacity = 0);
    ConstDataElement(dr_data_entry const & o);

    operator int8_t(void) const;
    operator uint8_t(void) const;
    operator int16_t(void) const;
    operator uint16_t(void) const;
    operator int32_t(void) const;
    operator uint32_t(void) const;
    operator int64_t(void) const;
    operator uint64_t(void) const;
    operator float(void) const;
    operator double(void) const;
    operator const char *(void) const;

    int8_t asInt8(void) const { return *this; }
    uint8_t asUInt8(void) const { return *this; }
    int16_t asInt16(void) const { return *this; }
    uint16_t asUInt16(void) const { return *this; }
    int32_t asInt32(void) const { return *this; }
    uint32_t asUInt32(void) const { return *this; }
    int64_t asInt64(void) const { return *this; }
    uint64_t asUInt64(void) const { return *this; }
    float asFloat(void) const { return *this; }
    double asDouble(void) const { return *this; }
    Cpe::Utils::CString const & asString(void) const { return Cpe::Utils::CString::_cast((const char *)(*this)); }

    ConstData operator[] (size_t pos) const;

    const void * data(void) const { return m_data; }
    size_t capacity(void) const { return m_capacity; }
    Entry const & entry(void) const { return *(Entry const *)m_entry; }
    Meta const & meta(void) const { return *(Meta const *)dr_entry_self_meta(m_entry); }

    template<typename T>
    T const & as(void) const { return *(T const *)data(); }

    template<typename T>
    T const * check_as(void) const { return Cpe::Dr::MetaTraits<T>::META == meta() ? (T const *)data() : NULL; }

protected:
    const void * m_data;
    size_t m_capacity;
    LPDRMETAENTRY m_entry;
};

class DataElement : public ConstDataElement {
public:
    DataElement(void * data, LPDRMETAENTRY entry, size_t capacity = 0);
    DataElement(dr_data_entry const & o);

    DataElement & operator=(int8_t d);
    DataElement & operator=(uint8_t d);
    DataElement & operator=(int16_t d);
    DataElement & operator=(uint16_t d);
    DataElement & operator=(int32_t d);
    DataElement & operator=(uint32_t d);
    DataElement & operator=(int64_t d);
    DataElement & operator=(uint64_t d);
    DataElement & operator=(float d);
    DataElement & operator=(double d);
    DataElement & operator=(const char * d);
    DataElement & operator=(ConstDataElement const & o);

    using ConstDataElement::operator[];
    Data operator[](size_t pos);

    using ConstDataElement::as;
    using ConstDataElement::check_as;
    
    template<typename T>
    T & as(void) { return *(T *)data(); }

    template<typename T>
    T * check_as(void) { return Cpe::Dr::MetaTraits<T>::META == meta() ? (T *)data() : NULL; }
    
    void copy(const void * data, size_t capacity);
    void copy(ConstData const & data);
    void copy(Data const & data);
    void copy(ConstDataElement const & data);
    void copy(DataElement const & data);

    template<typename T>
    void copy(T const & data) { copy(&data, sizeof(data)); }
};

class ConstData {
public:
    template<typename T>
    ConstData(T const & data)
        : m_data(&(const_cast<T &>(data)))
        , m_capacity(sizeof(T))
        , m_meta(MetaTraits<T>::META)
    {
    }

    ConstData(ConstData const & o)
        : m_data(o.m_data)
        , m_capacity(o.m_capacity)
        , m_meta(o.m_meta)
    {
    }

    ConstData(Data const & o);

    ConstData(const void * data, LPDRMETA meta = 0, size_t capacity = 0);
    ConstData(ConstDataElement const & e);
    ConstData(DataElement const & e);

    Meta const & meta(void) const { return *((Meta*)m_meta); }
    const void * data(void) const { return m_data; }
    size_t capacity(void) const { return m_capacity; }

    ConstDataElement operator[](const char * name) const;
    ConstDataElement operator[](LPDRMETAENTRY entry) const;

    template<typename T>
    T const & as(void) const { return *(T *)data(); }

    bool is_valid(void) const { return m_data ? true : false; }
    bool is_valid(const char * name) const;
    void dump_data(write_stream_t stream) const;
    const char * dump_data(mem_buffer_t buffer) const;

protected:
    const void * m_data;
    size_t m_capacity;
    LPDRMETA m_meta;
};

class Data : public ConstData {
public:
    template<typename T>
    Data(T & data) : ConstData(&data, MetaTraits<T>::META, sizeof(T)) {}
    Data(Data & o) : ConstData(o) {}
    Data(Data const & o) : ConstData(o) {}
    Data(void * data, LPDRMETA meta, size_t capacity = 0);
    Data(void * data, size_t capacity = 0);
    Data(DataElement const & element);

    using ConstData::data;
    void * data(void)  { return const_cast<void*>(m_data); }

    using ConstData::operator[];
    DataElement operator[](const char * name);
    DataElement operator[](LPDRMETAENTRY entry);

    void setMeta(LPDRMETA meta);
    void setCapacity(size_t capacity);
    void copySameEntriesFrom(ConstData const & o, error_monitor_t em = 0);
    void clear(void);

    void copy(const void * data, size_t capacity);
    void copy(ConstData const & data);
    void copy(Data const & data);
    void copy(ConstDataElement const & data);
    void copy(DataElement const & data);

    template<typename T>
    T & as(void) { return *(T *)data(); }

    template<typename T>
    T const & as(void) const { return *(T const *)data(); }

    template<typename T>
    T * check_as(void) { return Cpe::Dr::MetaTraits<T>::META == meta() ? (T *)data() : NULL; }

    template<typename T>
    T const * check_as(void) const { return Cpe::Dr::MetaTraits<T>::META == meta() ? (T const *)data() : NULL; }

private:
    Data(ConstDataElement const & e);
};

inline void DataElement::copy(ConstData const & data) { copy(data.data(), data.capacity()); }
inline void DataElement::copy(Data const & data) { copy(data.data(), data.capacity()); }
inline void DataElement::copy(ConstDataElement const & data) { copy(data.data(), data.capacity()); }
inline void DataElement::copy(DataElement const & data) { copy(data.data(), data.capacity()); }

inline void Data::copy(ConstData const & data) { copy(data.data(), data.capacity()); }
inline void Data::copy(Data const & data) { copy(data.data(), data.capacity()); }
inline void Data::copy(ConstDataElement const & data) { copy(data.data(), data.capacity()); }
inline void Data::copy(DataElement const & data) { copy(data.data(), data.capacity()); }

inline ConstData::ConstData(Data const & o)
    : m_data(o.m_data)
    , m_capacity(o.m_capacity)
    , m_meta(o.m_meta)
{
}

}}


#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
