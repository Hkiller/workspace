#ifndef GDPP_APP_MODULE_H
#define GDPP_APP_MODULE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gd/app/app_module.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)

#endif
namespace Gd { namespace App {

class Module:  public Cpe::Utils::SimulateObject {
public:
    operator gd_app_module_t () const { return (gd_app_module_t)(void*)this; }

    const char * name(void) const { return gd_app_module_name(*this); }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif

