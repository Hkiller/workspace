#ifndef CPE_CFT_TESTENV_WITH_CFG_H
#define CPE_CFT_TESTENV_WITH_CFG_H
#include "cpe/utils/error.h"
#include "cpe/utils/error_list.h"
#include "cpe/utils/tests-env/test-env.hpp"
#include "../cfg.h"

namespace cpe { namespace cfg { namespace testenv {

class with_cfg : public ::testenv::env<> {
public:
    cfg_t t_cfg_create(void);
    cfg_t t_cfg_parse(const char * input);
    void t_cfg_read(cfg_t cfg, const char * input);

    const char * t_cfg_dump(cfg_t cfg, int ident = 0, int level_ident = 4);
    const char * t_cfg_dump_inline(cfg_t cfg);
};

}}}

#define EXPECT_CFG_DO_CMP(__expect, __cfg, __policy)                    \
    do {                                                                \
        cfg_t cmpRoot = (__cfg);                                        \
        const char * str_expect = __expect;                             \
        cfg_t expectRoot = 0;                                           \
        if (str_expect && strlen(str_expect) > 0) {                     \
            expectRoot = t_cfg_parse(str_expect);                       \
            EXPECT_TRUE(expectRoot) << "parse expect cfg fail!";        \
            EXPECT_TRUE(cmpRoot) << "cmp target is fail!";              \
            if (cmpRoot && expectRoot) {                                \
                error_list_t el =                                       \
                    cpe_error_list_create(t_tmp_allocrator());          \
                struct error_monitor em =                               \
                    CPE_DEF_ERROR_MONITOR_INITIALIZER(                  \
                        cpe_error_list_collect, el);                    \
                int cmp_result = cfg_cmp(                               \
                    expectRoot, cmpRoot, __policy, &em);                \
                if (cmp_result != 0) {                                  \
                    struct mem_buffer buffer;                           \
                    mem_buffer_init(&buffer, t_tmp_allocrator());       \
                    EXPECT_EQ(0, cmp_result)                            \
                        << "expect:\n" << t_cfg_dump(expectRoot, 4)     \
                        << "\nactual:\n" << t_cfg_dump(cmpRoot, 4)      \
                        << "\ndetails:\n" << cpe_error_list_dump(el, &buffer, 4); \
                }                                                       \
            }                                                           \
        }                                                               \
        else {                                                          \
            if (cmpRoot) {                                              \
                EXPECT_TRUE(cmpRoot == 0)                               \
                    << "expect null, bug exist!\n"                      \
                    << "\nactual:\n" << t_cfg_dump(cmpRoot, 4)          \
                    ;                                                   \
            }                                                           \
        }                                                               \
    } while(0)

#define EXPECT_CFG_EQ(__expect, __cfg)          \
    EXPECT_CFG_DO_CMP(__expect, __cfg, 0)

#define EXPECT_CFG_EQ_PART(__expect, __cfg)                             \
    EXPECT_CFG_DO_CMP(__expect, __cfg,                                  \
                      CFG_CMP_POLICY_L_STRUCT_LEAK)

#endif
