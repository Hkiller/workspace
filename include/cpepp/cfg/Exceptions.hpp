#ifndef CPEPP_CFG_EXCEPTIONS_H
#define CPEPP_CFG_EXCEPTIONS_H
#include <stdexcept>
#include "System.hpp"

namespace Cpe { namespace Cfg {

class invalid_cfg_node : public ::std::runtime_error {
public:
    invalid_cfg_node(const char * msg) : ::std::runtime_error(msg) {}
};

}}

#endif

