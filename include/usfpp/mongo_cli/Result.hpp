#ifndef USFPP_MONGO_CLI_RESULT_H
#define USFPP_MONGO_CLI_RESULT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usf/mongo_cli/mongo_cli_result.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Mongo {

class Result : public Cpe::Utils::SimulateObject {
public:
    operator mongo_cli_result_t() const { return (mongo_cli_result_t)this; }

    int32_t n(void) const { return mongo_cli_result_n(*this); }
    int32_t code(void) const { return mongo_cli_result_code(*this); }

    static Result * find(logic_require_t requlre);
    static Result & get(logic_require_t requlre);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
