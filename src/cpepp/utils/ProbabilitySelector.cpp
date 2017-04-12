#include "cpepp/utils/ProbabilitySelector.hpp"

namespace Cpe { namespace Utils {

bool randomSelectIfNeed(int32_t probability, Cpe::Utils::Random & random) {
    if (probability == 0) {
        return false;
    }
    else {
        return probability == 10000 || (int32_t)random.generate(10000) < probability;
    }
}

}}
