#ifndef CPEPP_DR_EXCEPTIONS_H
#define CPEPP_DR_EXCEPTIONS_H
#include <stdexcept>
#include "System.hpp"

namespace Cpe { namespace Dr {

class type_convert_error : public ::std::runtime_error {
public:
    type_convert_error(const char * msg) : ::std::runtime_error(msg) {}
};


}}

#endif
