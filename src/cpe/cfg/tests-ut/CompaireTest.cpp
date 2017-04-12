#include "CompaireTest.hpp"

int CompaireTest::compaire(const char * l, const char * r, int policy) {
    return cfg_cmp(t_cfg_parse(l), t_cfg_parse(r), policy, t_em());
}

