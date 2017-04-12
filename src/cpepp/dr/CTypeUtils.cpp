#include <stdexcept>
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpepp/dr/CTypeUtils.hpp"

namespace Cpe { namespace Dr {

void dr_ctype_set_from_string_check_throw(void * output, int type, const char * input) {
    if (dr_ctype_set_from_string(output, type, input, NULL) != 0) {
        ::std::ostringstream os;
        os << "type " << dr_type_name(type) << " from " << input << " fail!";
        throw ::std::runtime_error(os.str());
    }
}

}}
