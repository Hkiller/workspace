#ifndef CPEPP_DR_UTILS_H
#define CPEPP_DR_UTILS_H
#include "Meta.hpp"

namespace Cpe { namespace Dr {

template<class T>
inline Meta const & metaOf(T const & data) { return MetaTraits<T>::META; }

template<class T>
inline Meta const & metaOf(T const * data) { return MetaTraits<T>::META; }

template<class T>
size_t calc_dyn_size(size_t record_count) { return MetaTraits<T>::META.calc_dyn_size(record_count); }

template<class T>
size_t data_size(T const & o) { return MetaTraits<T>::data_size(o); }

template<class T1, typename T2>
inline void copy_same_entries(T1 & target, T2 const & src, int policy = 0, error_monitor_t em = 0) {
    MetaTraits<T1>::META.copy_same_entries(
        &target, sizeof(target),
        &src, MetaTraits<T2>::META,
        sizeof(src), policy, em);
}

template<class T1, typename T2>
inline void copy_same_entries_part(T1 & target, T2 const & src, const char * columns, int policy = 0, error_monitor_t em = 0) {
    MetaTraits<T1>::META.copy_same_entries_part(
        &target, sizeof(target),
        &src, MetaTraits<T2>::META,
        columns,
        sizeof(src), policy, em);
}

template<class T>
inline void set_defaults(T & data, int policy = 0) {
    MetaTraits<T>::META.set_defaults(&data, sizeof(data), policy);
}

template<class T>
inline void load_from_json(T & data, const char * json) {
    MetaTraits<T>::META.load_from_json(&data, sizeof(data), json);
}

template<typename T>
inline bool try_load_from_json(T & data, const char * json, error_monitor_t em = NULL) {
    return MetaTraits<T>::META.try_load_from_json(&data, sizeof(data), json, em);
}

template<class T>
inline void load_from_pbuf(T & data, const void * pbuf, size_t capacity) {
    MetaTraits<T>::META.load_from_pbuf(&data, sizeof(data), pbuf, capacity);
}

template<class T>
inline void load_from_pbuf(T & data, size_t data_capacity, const void * pbuf, size_t capacity) {
    MetaTraits<T>::META.load_from_pbuf(&data, data_capacity, pbuf, capacity);
}

template<typename T>
inline bool try_load_from_pbuf(T & data, const void * pbuf, size_t capacity, error_monitor_t em = NULL) {
    return MetaTraits<T>::META.try_load_from_pbuf(&data, sizeof(data), pbuf, capacity, em);
}

template<class T>
inline size_t write_to_pbuf(void * pbuf, size_t capacity, T const & data) {
    return MetaTraits<T>::META.write_to_pbuf(pbuf, capacity, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
}

template<class T>
inline void load_from_cfg(T & data, cfg_t cfg, int policy = DR_CFG_READ_CHECK_NOT_EXIST_ATTR) {
    MetaTraits<T>::META.load_from_cfg(&data, sizeof(data), cfg, policy);
}

template<class T>
inline bool try_load_from_cfg(T & data, cfg_t cfg, error_monitor_t em = 0, int policy = 0) {
    return MetaTraits<T>::META.try_load_from_cfg(&data, sizeof(data), cfg, em, policy);
}

template<typename T>
inline const char * dump_data(mem_buffer_t buffer, T const & data) {
    return MetaTraits<T>::META.dump_data(buffer, &data, MetaTraits<T>::data_size(data));
}

template<typename T>
inline void dump_data(write_stream_t stream, T const & data) {
    MetaTraits<T>::META.dump_data(stream, &data, MetaTraits<T>::data_size(data));
}

template<typename T>
inline void write_to_cfg(cfg_t cfg, T const & data) {
    MetaTraits<T>::META.write_to_cfg(cfg, &data);
}

template<typename T>
inline bool try_write_to_cfg(cfg_t cfg, T const & data,  error_monitor_t em = 0) {
    return MetaTraits<T>::try_write_to_cfg(cfg, &data, em);
}

}}

#endif

