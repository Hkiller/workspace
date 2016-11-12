#include <limits>
#include <cassert>
#include "cpepp/utils/Random.hpp"

namespace Cpe { namespace Utils {

Random::~Random() {
}

uint32_t Random::generate(void) {
    return generate(::std::numeric_limits<uint32_t>::max());
}

double Random::generateBetween(double min, double max, int calc_p) {
    int32_t minI = (int32_t)(min * calc_p);
    int32_t maxI = (int32_t)(max * calc_p);

    assert(calc_p > 0);

    if(maxI < minI ) {
        maxI = minI;
    }

    int32_t result = (int32_t)( minI + this->generate(maxI - minI + 1));

    return result / (double)calc_p;
}

float Random::generateBetween(float min, float max, int calc_p) {
    int32_t minI = (int32_t)(min * calc_p);
    int32_t maxI = (int32_t)(max * calc_p);

    assert(calc_p > 0);

    if(maxI < minI ) {
        maxI = minI;
    }

    int32_t result = (int32_t)( minI + this->generate(maxI - minI + 1));

    return result / (float)calc_p;
}

RandomAdapter::RandomAdapter(cpe_rand_ctx & ctx)
    : m_ctx(ctx)
{
}

uint32_t RandomAdapter::generate(uint32_t max) {
    return cpe_rand_ctx_generate(&m_ctx, max);
}

static RandomAdapter s_ins(*cpe_rand_ctx_dft());

Random &
Random::dft(void) {
    return s_ins;
}

}}
