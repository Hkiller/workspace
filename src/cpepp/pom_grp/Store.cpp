#include <limits>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include "cpe/pal/pal_stdio.h"
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpepp/pom_grp/Store.hpp"

namespace Cpe { namespace PomGrp {

Store & Store::_cast(pom_grp_store_t store) {
    if (store == NULL) {
        throw ::std::runtime_error("store is NULL!"); 
    }

    return *reinterpret_cast<Store*>(store);
}

StoreTable const & Store::table(const char * name) {
    StoreTable const * r = findTabke(name);
    if (r == NULL) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Table %s not exist in store!", name); 
        throw ::std::runtime_error(buf);
    }
    return *r;
}

}}
