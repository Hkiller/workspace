#include <stdexcept>
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/mongo_cli/Result.hpp"

namespace Usf { namespace Mongo {

Result * Result::find(logic_require_t requlre) {
    return (Result*)mongo_cli_result_find(requlre);
}

Result & Result::get(logic_require_t requlre) {
    Result * r = find(requlre);
    if (r == NULL) {
        throw ::std::runtime_error("Usf::Mongo::Result::get: mongo result not exist in require!");
    }

    return *r;
}

}}
