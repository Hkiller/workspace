#include "WeightSelectorTest.hpp"

void WeightSelectorTest::add(uint32_t v) {
    m_selector.addWeightItem(v);
}

int32_t WeightSelectorTest::select() {
    return m_selector.select(t_random_mock());
}
