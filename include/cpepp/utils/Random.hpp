#ifndef CPEPP_UTILS_RANDOM_H
#define CPEPP_UTILS_RANDOM_H
#include "cpe/utils/random.h"
#include "System.hpp"

namespace Cpe { namespace Utils {

class Random {
public:
    uint32_t generate(void);
    virtual uint32_t generate(uint32_t max) = 0;
    virtual ~Random();

	//取一个范围里的值，两遍闭区间，两位精度
	float generateBetween(float min, float max, int calc_p = 1000);
	double generateBetween(double min, double max, int calc_p = 1000);

    static Random & dft(void);
};

class RandomAdapter : public Random {
public:
    RandomAdapter(cpe_rand_ctx & ctx);
    virtual uint32_t generate(uint32_t max);

private:
    cpe_rand_ctx & m_ctx;
};

}}

#endif

