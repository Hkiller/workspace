#ifndef CPEPP_POM_GRP_STORE_H
#define CPEPP_POM_GRP_STORE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/pom_grp/pom_grp_store.h"
#include "cpepp/dr/System.hpp"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace PomGrp {

class Store : public Cpe::Utils::SimulateObject {
public:
    operator pom_grp_store_t (void) const { return (pom_grp_store_t)(this); }

    StoreTable const * findTabke(const char * name) const {
        return (StoreTable *)pom_grp_store_table_find(*this, name);
    }

    StoreTable const & table(const char * name);

    static Store & _cast(pom_grp_store_t store);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
