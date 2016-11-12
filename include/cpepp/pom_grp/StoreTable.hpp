#ifndef CPEPP_POM_GRP_STORE_TABLE_H
#define CPEPP_POM_GRP_STORE_TABLE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/Meta.hpp"
#include "cpe/pom_grp/pom_grp_store.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace PomGrp {

class StoreTable : public Cpe::Utils::SimulateObject {
public:
    operator pom_grp_store_table_t (void) const { return (pom_grp_store_table_t)(this); }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(pom_grp_store_table_name(*this)); }
    Cpe::Dr::Meta const & meta(void) const { return Cpe::Dr::Meta::_cast(pom_grp_store_table_meta(*this)); }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
