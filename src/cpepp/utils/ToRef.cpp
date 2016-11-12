#include <stdexcept>
#include "cpe/pal/pal_stdio.h"
#include "cpepp/utils/ToRef.hpp"

namespace Cpe { namespace Utils {

void __throw_ptr_is_null_exception(const char * typeName) {
    char buf[128];
    snprintf(buf, sizeof(buf), "point of type %s is null!", typeName); 
}

}}
