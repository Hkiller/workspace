#ifndef CPE_UTILS_PRAND_H
#define CPE_UTILS_PRAND_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct cpe_prand_ctx * cpe_prand_ctx_t;

struct cpe_prand_ctx {
    double (*m_f)(void * ctx);
    char m_data[sizeof(double) * 200];
};

double cpe_prand(cpe_prand_ctx_t ctx);


/*prand序列用于生成在(0,1)上的随机数序列
  基础的prand_0,prand_1,prand_2,prand_3是基础的均匀分布随机数发生器
      cpe_prand_0用于改善系统提供的均匀分布随机数发生器rand，改进系统随机数的序列相关性
      cpe_prand_1用三个线性同余，可以认为是周期无穷大的随机数序列
      cpe_prand_2用一个线性同余，计算速度较快
      cpe_prand_3用减法，性能最快
  变换方法是在基础的随机数发生器基础上，叠加计算，生成一个符合一定分布规律的随机数序列
      cpe_prand_expdev 指数分布的随机数
      cpe_prand_gasdev 正态分布（高斯分布）
*/

void cpe_prand_init_basic_1(cpe_prand_ctx_t ctx, int32_t idum);

void cpe_prand_init_expdev(cpe_prand_ctx_t ctx, int32_t seed);
void cpe_prand_init_gasdev(cpe_prand_ctx_t ctx, int32_t seed);

#ifdef __cplusplus
}
#endif

#endif
