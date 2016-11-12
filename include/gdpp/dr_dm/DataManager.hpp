#ifndef GDPP_DR_DM_ROLEMANAGER_H
#define GDPP_DR_DM_ROLEMANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "gd/dr_dm/dr_dm_manage.h"
#include "gd/dr_dm/dr_dm_data.h"
#include "System.hpp"
#include "DataIterator.hpp"

namespace Gd { namespace Dr {

class DataManager : public Cpe::Utils::SimulateObject {
public:
    operator dr_dm_manage_t() const { return (dr_dm_manage_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(dr_dm_manage_name(*this)); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(dr_dm_manage_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(dr_dm_manage_app(*this)); }

    Cpe::Dr::Meta const & dataMeta(void) const { return Cpe::Dr::Meta::_cast(dr_dm_manage_meta(*this)); }

    DataIterator datas(void);
    DataConstIterator datas(void) const;

    /*create*/
    Data * createData(const void * data, size_t size, const char ** duplicate_index) {
        dr_dm_data_t r = dr_dm_data_create(*this, (void*)data, size, duplicate_index);
        return (Data *)r;
    }

    Data & createData(const void * data, size_t size);

    template<typename T>
    T * createData(T const & input, const char ** duplicate_index) { 
        Data * d = DataManager::createData(&input, sizeof(input), duplicate_index);
        return d ? (T*)dr_dm_data_data((dr_dm_data_t)d) : NULL;
    }

    template<typename T>
    T & createData(T const & input) { 
        Data & d = DataManager::createData(&input, sizeof(input));
        return *(T*)dr_dm_data_data((dr_dm_data_t)&d);
    }
    
    /*by id*/
    Data const * findData(dr_dm_data_id_t id) const { return (Data*)dr_dm_data_find_by_id(*this, id); }
    Data * findData(dr_dm_data_id_t id) { return (Data*)dr_dm_data_find_by_id(*this, id); }

    /*by index int8*/
    Data * findData(const char * index, int8_t key) { return (Data*)dr_dm_data_find_by_index_int8(*this, index, key); }
    Data const * findData(const char * index, int8_t key) const { return (Data const *)dr_dm_data_find_by_index_int8(*this, index, key); }

    /*by index uint8*/
    Data * findData(const char * index, uint8_t key) { return (Data*)dr_dm_data_find_by_index_uint8(*this, index, key); }
    Data const * findData(const char * index, uint8_t key) const { return (Data const *)dr_dm_data_find_by_index_uint8(*this, index, key); }

    /*by index int16*/
    Data * findData(const char * index, int16_t key) { return (Data*)dr_dm_data_find_by_index_int16(*this, index, key); }
    Data const * findData(const char * index, int16_t key) const { return (Data const *)dr_dm_data_find_by_index_int16(*this, index, key); }

    /*by index uint16*/
    Data * findData(const char * index, uint16_t key) { return (Data*)dr_dm_data_find_by_index_uint16(*this, index, key); }
    Data const * findData(const char * index, uint16_t key) const { return (Data const *)dr_dm_data_find_by_index_uint16(*this, index, key); }

    /*by index int32*/
    Data * findData(const char * index, int32_t key) { return (Data*)dr_dm_data_find_by_index_int32(*this, index, key); }
    Data const * findData(const char * index, int32_t key) const { return (Data const *)dr_dm_data_find_by_index_int32(*this, index, key); }

    /*by index uint32*/
    Data * findData(const char * index, uint32_t key) { return (Data*)dr_dm_data_find_by_index_uint32(*this, index, key); }
    Data const * findData(const char * index, uint32_t key) const { return (Data const *)dr_dm_data_find_by_index_uint32(*this, index, key); }

    /*by index int64*/
    Data * findData(const char * index, int64_t key) { return (Data*)dr_dm_data_find_by_index_int64(*this, index, key); }
    Data const * findData(const char * index, int64_t key) const { return (Data const *)dr_dm_data_find_by_index_int64(*this, index, key); }

    /*by index uint64*/
    Data * findData(const char * index, uint64_t key) { return (Data*)dr_dm_data_find_by_index_uint64(*this, index, key); }
    Data const * findData(const char * index, uint64_t key) const { return (Data const *)dr_dm_data_find_by_index_uint64(*this, index, key); }

    /*by index float*/
    Data * findData(const char * index, float key) { return (Data*)dr_dm_data_find_by_index_float(*this, index, key); }
    Data const * findData(const char * index, float key) const { return (Data const *)dr_dm_data_find_by_index_float(*this, index, key); }

    /*by index double*/
    Data * findData(const char * index, double key) { return (Data*)dr_dm_data_find_by_index_double(*this, index, key); }
    Data const * findData(const char * index, double key) const { return (Data const *)dr_dm_data_find_by_index_double(*this, index, key); }

    /*by index string*/
    Data * findData(const char * index, const char * key) { return (Data*)dr_dm_data_find_by_index_string(*this, index, key); }
    Data const * findData(const char * index, const char * key) const { return (Data const *)dr_dm_data_find_by_index_string(*this, index, key); }

    template<typename T, typename KeyT>
    T * findData(const char * index, KeyT key) { 
        Data * d = DataManager::findData(index, key);
        return d ? (T*)dr_dm_data_data((dr_dm_data_t)d) : NULL;
    }

    template<typename T, typename KeyT>
    T const * findData(const char * index, KeyT key)  const { 
        Data const * d = DataManager::findData(index, key);
        return d ? (T const *)dr_dm_data_data((dr_dm_data_t)d) : NULL;
    }

    template<typename T>
    T * findData(dr_dm_data_id_t id) { 
        Data * d = DataManager::findData(id);
        return d ? (T*)dr_dm_data_data((dr_dm_data_t)d) : NULL;
    }

    template<typename T>
    T const * findData(dr_dm_data_id_t id) const { 
        Data const * d = DataManager::findData(id);
        return d ? (T const *)dr_dm_data_data((dr_dm_data_t)d) : NULL;
    }

    static DataManager & _cast(dr_dm_manage_t data_manage);
    static DataManager & instance(gd_app_context_t app, const char * name);
};

}}

#endif
