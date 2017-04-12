#ifndef CPE_UTILS_WITH_EM_H
#define CPE_UTILS_WITH_EM_H
#include "../error.h"
#include "../error_list.h"
#include "test-env.hpp"

namespace utils { namespace testenv {

class with_em : public ::testenv::env<> {
public:
    void SetUp();
    void TearDown();

    error_monitor_t t_em();
    error_list_t t_elist();
    void t_em_set_print();

    void t_elist_clear(void);

    int t_em_no_error(void);
    int t_em_error_count(void);
    int t_em_have_errno(int e);

private:
    error_list_t m_el;
    struct error_monitor m_em;
    struct error_monitor_node m_printer;
};

}}

#endif

