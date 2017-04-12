#ifndef GDPP_EVT_EXCEPTIONS_H
#define GDPP_EVT_EXCEPTIONS_H
#include <stdexcept>
#include "System.hpp"

namespace Gd { namespace Evt {

class no_responser_error : public ::std::runtime_error {
public:
    no_responser_error(const char * msg) : ::std::runtime_error(msg) {}
};

}}

#endif
