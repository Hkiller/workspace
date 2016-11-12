#include <limits>
#include <algorithm>
#include "cpepp/utils/WeightSelector.hpp"

namespace Cpe { namespace Utils {

void WeightSelector::addWeightItem(uint32_t weight) {
    if (m_weights.empty()) {
        m_weights.push_back(weight);
    }
    else {
        m_weights.push_back(m_weights.back() + weight);
    }
}

void WeightSelector::clear(void) {
    m_weights.clear();
}

int32_t WeightSelector::select(Random & random) const {
    if (m_weights.empty()) return -1;

    if (m_weights.back() > 0) {
        uint32_t rv = random.generate(m_weights.back());

        ::std::vector<uint32_t>::const_iterator pos =
              ::std::upper_bound(m_weights.begin(), m_weights.end(), rv);

        return pos == m_weights.end()
            ? -1
            : (int)(pos - m_weights.begin());
    }
    else {
        uint32_t rv = random.generate((uint32_t)m_weights.size());
        return rv >= m_weights.size()
            ? -1
            : (int32_t)rv;
    }
}

}}
