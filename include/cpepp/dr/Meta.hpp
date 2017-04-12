#ifndef CPEPP_DR_META_H
#define CPEPP_DR_META_H
#include "cpe/utils/stream.h"
#include "cpe/utils/buffer.h"
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Dr {

class Meta : public Cpe::Utils::SimulateObject {
public:
    operator LPDRMETA (void) const { return (LPDRMETA)this; }

    MetaLib const & ownerLib(void) const { return *(MetaLib const *)dr_meta_owner_lib(*this); }

    int type(void) const { return dr_meta_type(*this); }

    int baseVersion(void) const { return dr_meta_based_version(*this); }
    int currentVersion(void) const { return dr_meta_current_version(*this); }
 
    Utils::CString const & name(void) const { return Utils::CString::_cast(dr_meta_name(*this)); } 
    Utils::CString const & desc(void) const { return Utils::CString::_cast(dr_meta_desc(*this)); } 
    int id(void) const { return dr_meta_id(*this); } 

    bool isDynamic(void) const;
    LPDRMETA recordMeta(void) const;

    size_t size(void) const { return dr_meta_size(*this); } 
    int align(void) const { return dr_meta_align(*this); } 

    int entryCount(void) const { return dr_meta_entry_num(*this); } 
    Entry const & entryAt(int idx) const;

    int findEntryIdx(const char * name) const { return dr_meta_find_entry_idx_by_name(*this, name); }
    int findEntryIdx(int id) const { return dr_meta_find_entry_idx_by_id(*this, id); }
    int entryIdx(const char * name) const;
    int entryIdx(int id) const;

    Entry const * findEntry(const char * name) const { return (Entry const *)dr_meta_find_entry_by_name(*this, name); }
    Entry const * findEntry(int id) const;
    Entry const & entry(const char * name) const;
    Entry const & entry(int id) const;

    Entry const * findEntryByPath(const char * path) const { return (Entry const*)dr_meta_find_entry_by_path(*this, path); }
    Entry const & entryByPath(const char * path) const;

    Entry const * lsearchEntryByType(const char * type_name) const { return (Entry const*)dr_meta_lsearch_entry_by_type_name(*this, type_name); }

    void dump_data(write_stream_t stream, const void * data, size_t capacity) const;
    const char * dump_data(mem_buffer_t buffer, const void * data, size_t capacity) const;

    void dump_data_array(write_stream_t stream, const void * data, size_t capacity) const;
    const char * dump_data_array(mem_buffer_t buffer, const void * data, size_t capacity) const;

    void set_defaults(void * data, size_t capacity, int policy = 0) const;

    template<typename T>
    void set_defaults(T & data, int policy = 0) const { set_defaults(&data, sizeof(data), policy); }

    void copy_same_entries(
        void * data, size_t capacity, const void * src, LPDRMETA srcMeta,
        size_t srcCapacity = 0, int policy = 0, error_monitor_t em = 0) const;
    void copy_same_entries(
        void * data, size_t capacity, const void * src, const char * srcMeta,
        size_t srcCapacity = 0, int policy = 0, error_monitor_t em = 0) const;

    void copy_same_entries_part(
        void * data, size_t capacity, const void * src, LPDRMETA srcMeta, const char * columns,
        size_t srcCapacity = 0, int policy = 0, error_monitor_t em = 0) const;
    void copy_same_entries_part(
        void * data, size_t capacity, const void * src, const char * srcMeta, const char * columns,
        size_t srcCapacity = 0, int policy = 0, error_monitor_t em = 0) const;

    /*operations of data format json*/
    void load_from_json(void * data, size_t capacity, const char * json) const;
    bool try_load_from_json(void * data, size_t capacity, const char * json, error_monitor_t em = 0) const;

    /*operations of data format pbuf*/
    size_t write_to_pbuf(void * pbuf, size_t capacity, const void * data, size_t data_size) const;
    void load_from_pbuf(void * data, size_t capacity, const void * pbuf, size_t pbuf_size) const;
    bool try_load_from_pbuf(void * data, size_t capacity, const void * pbuf, size_t pbuf_size, error_monitor_t em = 0) const;

    size_t calc_dyn_size(size_t record_count) const;

    /*operations of data format cfg*/
    void load_from_cfg(void * data, size_t capacity, cfg_t cfg, int policy = DR_CFG_READ_CHECK_NOT_EXIST_ATTR) const;
    bool try_load_from_cfg(void * data, size_t capacity, cfg_t cfg, error_monitor_t em = 0, int policy = 0) const;

    template<typename T>
    void load_from_cfg(T & data, cfg_t cfg, int policy = DR_CFG_READ_CHECK_NOT_EXIST_ATTR) const {
        load_from_cfg(&data, sizeof(data), cfg);
    }

    template<typename T>
    bool try_load_from_cfg(T & data, cfg_t cfg, error_monitor_t em = 0, int policy = 0) const {
        return try_load_from_cfg(&data, sizeof(data), cfg);
    }

    void write_to_cfg(cfg_t cfg, const void * data) const;
    bool try_write_to_cfg(cfg_t cfg, const void * data,  error_monitor_t em = 0) const;

    template<typename T>
    void write_to_cfg(cfg_t cfg, T const & data) const {
        write_to_cfg(cfg, (const void *)&data);
    }

    template<typename T>
    bool try_write_to_cfg(cfg_t cfg, T const & data, error_monitor_t em = 0) const {
        return try_write_to_cfg(cfg, (const void *)&data, em);
    }

    static Meta const & _cast(LPDRMETA meta);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
