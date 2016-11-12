#include "cpe/pal/pal_math.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/prand.h"

class PRandBasic1 : public testenv::fixture<> {
public:
    virtual void SetUp() {
        Base::SetUp();

        cpe_prand_init_basic_1(&m_ctx, -1);
    }

    double fnc(double x1, double x2, double x3, double x4) {
        return sqrt(x1 * x1 + x2 * x2 + x3 * x3 + x4 * x4);
    }

    cpe_prand_ctx m_ctx;
};

TEST_F(PRandBasic1, basic) {
    double yprob[4];

    double a = 0.0;
    double b = 0.0;
    int iy[4] = { 0 };

    struct {
        double r_1;
        double r_2;
        double r_3;
    } expects[] = {
        { 4.0, 8.0, 8.0 }
        , { 5.0, 6.0, 8.0 }
        , { 4.5, 5.0, 6.0 }
        , { 3.75, 4.5, 4.0 }
        , { 3.25, 4.5, 5.5 }
    };

    for(int j = 1; j <= 15; ++j) {
        for(double k = pow(2, (j - 1)); k <= pow(2, j); k++) {
            double x1 = cpe_prand(&m_ctx);
            double x2 = cpe_prand(&m_ctx);
            double x3 = cpe_prand(&m_ctx);
            double x4 = cpe_prand(&m_ctx);

            if (fnc(x1, x2, a, b) < 1.0) iy[1] = iy[1] + 1;
            if (fnc(x1, x2, x3, 1) < 1.0) iy[2] = iy[2] + 1;
            if (fnc(x1, x2, x3, x4) < 1.0) iy[3] = iy[3] + 1;
        }

        for(int i = 1; i <= 3; ++i) {
            yprob[i] = 1.0 * pow(2, (i + 1)) * iy[i] / pow(2, j);
        }

        // EXPECT_DOUBLE_EQ(expects[j - 1].r_1, yprob[1]) << pow(2, j) << ": pi expect error";
        // EXPECT_DOUBLE_EQ(expects[j - 1].r_2, yprob[2]) << pow(2, j) << ": (4/3) * pi expect error";
        // EXPECT_DOUBLE_EQ(expects[j - 1].r_3, yprob[3]) << pow(2, j) << ": (1/2) * pi ^ 2 expect error";
    }
}
