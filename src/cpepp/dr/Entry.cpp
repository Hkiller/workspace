#include <sstream>
#include <stdexcept>
#include "cpepp/dr/Entry.hpp"

namespace Cpe { namespace Dr {

Entry const & Entry::_cast(LPDRMETAENTRY entry) {
    if (entry == NULL) {
        throw ::std::runtime_error("cast to Entry: input is NULL");
    }
    return *(Entry const *)entry;
}

}}
