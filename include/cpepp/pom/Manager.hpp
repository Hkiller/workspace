#ifndef CPEPP_POM_MANAGE_H
#define CPEPP_POM_MANAGE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/pom/pom_manage.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Pom {

class Manager : public Cpe::Utils::SimulateObject {
public:
    operator pom_mgr_t (void) const { return (pom_mgr_t)(this); }

    void * alloc(cpe_hash_string_t className);
    void * alloc_nothrow(cpe_hash_string_t className);
    void free(Object * o);

    void registClass(const char * className, size_t object_size, size_t align = sizeof(int));

    void registClass(cpe_hash_string_t className, size_t object_size, size_t align = sizeof(int)) {
        registClass(cpe_hs_data(className), object_size, align);
    }

    template<typename T>
    T * alloc(void) {
        return reinterpret_cast<T*>(this->alloc(T::CLASS_NAME));
    }

    template<typename T>
    void registClass(size_t align = sizeof(int)) {
        this->registClass(T::CLASS_NAME, sizeof(T), align);
    }

    static Manager & _cast(pom_mgr_t omm);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
