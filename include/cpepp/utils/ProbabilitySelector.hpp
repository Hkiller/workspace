#ifndef CPEPP_UTILS_PROBABILITYSELECTOR_H
#define CPEPP_UTILS_PROBABILITYSELECTOR_H
#include "cpe/pal/pal_types.h"
#include "cpepp/utils/Random.hpp"

namespace Cpe { namespace Utils {

bool randomSelectIfNeed(int32_t probability, Cpe::Utils::Random & random);

}}

#endif
