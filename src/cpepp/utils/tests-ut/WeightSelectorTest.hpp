#ifndef CPEPP_UTILS_TEST_WEIGHTSELECTOR_H
#define CPEPP_UTILS_TEST_WEIGHTSELECTOR_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpepp/utils/tests-env/with_random.hpp"
#include "cpepp/utils/WeightSelector.hpp"

typedef LOKI_TYPELIST_1(
    Cpe::Utils::testenv::with_random) WeightSelectorTestBase;

class WeightSelectorTest : public testenv::fixture<WeightSelectorTestBase> {
public:
    void add(uint32_t v);
    int32_t select(void);
    
    Cpe::Utils::WeightSelector m_selector;
};

#endif
