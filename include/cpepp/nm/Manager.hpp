#ifndef GDPP_NM_MANAGE_H
#define GDPP_NM_MANAGE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/nm/nm_manage.h"
#include "System.hpp"
#include "ObjectIterator.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Nm {

class Manager : public Cpe::Utils::SimulateObject {
public:
    operator nm_mgr_t (void) const { return (nm_mgr_t)(this); }

    ObjectIterator objects(void);
    ConstObjectIterator objects(void) const;

    Object const * findObject(cpe_hash_string_t name) const;
    Object * findObject(cpe_hash_string_t name);
    Object const & object(cpe_hash_string_t name) const;
    Object & object(cpe_hash_string_t name);

    Object const * findObjectNc(const char * name) const;
    Object * findObjectNc(const char * name);
    Object const & objectNc(const char * name) const;
    Object & objectNc(const char * name);

    bool removeObject(cpe_hash_string_t name);
    bool removeObject(const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
