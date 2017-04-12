#ifndef CPEPP_UTILS_WEIGHTSELECTOR_H
#define CPEPP_UTILS_WEIGHTSELECTOR_H
#include <vector>
#include "cpe/pal/pal_types.h"
#include "Random.hpp"

namespace Cpe { namespace Utils {

class WeightSelector {
public:
    void addWeightItem(uint32_t weight);
    void clear(void);

    int32_t select(Random & random) const;

private:
    ::std::vector<uint32_t> m_weights;
};

}}

#endif

