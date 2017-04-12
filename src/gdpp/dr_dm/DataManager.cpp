#include "gdpp/app/Log.hpp"
#include "gdpp/dr_dm/Data.hpp"
#include "gdpp/dr_dm/DataManager.hpp"
#include "gdpp/dr_dm/DataIterator.hpp"

namespace Gd { namespace Dr {

DataIterator DataManager::datas(void) {
    DataIterator r;
    dr_dm_data_it_init(&r.m_it, *this);
    return r;
}

DataConstIterator DataManager::datas(void) const {
    DataConstIterator r;
    dr_dm_data_it_init(&r.m_it, *this);
    return r;
}

Data & DataManager::createData(const void * data, size_t size) {
    const char * duplicate_index;
    dr_dm_data_t r = dr_dm_data_create(*this, data, size, &duplicate_index);

    if (r == NULL) {
        if (duplicate_index) {
            APP_CTX_THROW_EXCEPTION(
                app(),
                ::std::runtime_error,
                "%s: create data: %s duplicate!", name().c_str(), duplicate_index);
        }
        else {
            APP_CTX_THROW_EXCEPTION(
                app(),
                ::std::runtime_error,
                "%s: create data: unknown error!", name().c_str());
        }
    }

    return *(Data*)r;
}

DataManager & DataManager::_cast(dr_dm_manage_t role_manage) {
    if (role_manage == NULL) {
        throw ::std::runtime_error("Gd::Dr::DataManager::_cast: input role_manage is NULL!");
    }

    return *(DataManager*)role_manage;
}

DataManager & DataManager::instance(gd_app_context_t app, const char * name) {
    dr_dm_manage_t role_manage = dr_dm_manage_find_nc(app, name);
    if (role_manage == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "role_manage %s not exist!", name ? name : "default");
    }

    return *(DataManager*)role_manage;
}

}}
