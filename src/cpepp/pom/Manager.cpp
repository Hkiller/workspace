#include <cassert>
#include <sstream>
#include <stdexcept>
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpe/pom/pom_object.h"
#include "cpepp/pom/Manager.hpp"

namespace Cpe { namespace Pom {

void * Manager::alloc(cpe_hash_string_t className) {
    pom_oid_t oid = pom_obj_alloc(*this, className, NULL);
    if (oid == POM_INVALID_OID) {
        throw ::std::bad_alloc();
    }

    void * p = pom_obj_get(*this, oid, NULL);
    if (p == NULL) {
        throw ::std::runtime_error("OmManager: get buf from oid fail!");
    }

    return p;
}

void * Manager::alloc_nothrow(cpe_hash_string_t className) {
    pom_oid_t oid = pom_obj_alloc(*this, className, NULL);
    if (oid == POM_INVALID_OID) {
        return NULL;
    }

    return pom_obj_get(*this, oid, NULL);
}

void Manager::free(Object * p) {
    pom_oid_t oid = pom_obj_id_from_addr(*this, (void*)p, NULL);
    assert(oid != POM_INVALID_OID);
    pom_obj_free(*this, oid, NULL);
}

void Manager::registClass(const char * className, size_t object_size, size_t align) {
    Cpe::Utils::ErrorCollector ec;
    pom_mgr_add_class(*this, className, object_size, align, ec);

    ec.checkThrowWithMsg< ::std::runtime_error>();
}

Manager & Manager::_cast(pom_mgr_t omm) {
    if (omm == NULL) {
        throw ::std::runtime_error("omm is NULL!"); 
    }

    return *reinterpret_cast<Manager*>(omm);
}

}}
