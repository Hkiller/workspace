#include "cpepp/utils/tests-env/with_random.hpp"
#include "gdpp/app/Random.hpp"
#include "gdpp/app/tests-env/with_app.hpp"

namespace Gd { namespace App { namespace testenv {

with_app::with_app() {
}

void with_app::SetUp() {
    gd::app::testenv::with_app::SetUp();
}

void with_app::TearDown() {
    gd::app::testenv::with_app::TearDown();
}

class RandomAdapter : public Random {
public:
    RandomAdapter(Cpe::Utils::Random & random)
        : m_random(random)
    {
    }

    virtual uint32_t generate(uint32_t max) {
        return m_random.generate(max);
    }

    Cpe::Utils::Random & m_random;
};

void with_app::t_app_install_random(const char * name) {
    t_app_install_random(
        envOf<Cpe::Utils::testenv::with_random>().t_random_mock(),
        name);
}

void with_app::t_app_install_random(Cpe::Utils::Random & random, const char * name) {
    if (name == 0) {
        name = cpe_hs_data(Random::DEFAULT_NAME);
    }

    new (gd_app_nm_mgr(t_app()), name) RandomAdapter(random);
}

}}}

