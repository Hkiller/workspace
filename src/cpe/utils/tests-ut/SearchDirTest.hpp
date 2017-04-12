#ifndef CPE_DR_TEST_SEARCHDIRTEST_H
#define CPE_DR_TEST_SEARCHDIRTEST_H
#include "gmock/gmock.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_file.hpp"
#include "cpe/utils/tests-env/with_em.hpp"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_file
    , utils::testenv::with_em) SearchDirTestBase;

class SearchDirTest : public testenv::fixture<SearchDirTestBase> {
public:
    class VisitCheckMock {
    public:
        MOCK_METHOD1(on_file, dir_visit_next_op_t(const char *));
        MOCK_METHOD1(on_enter_dir, dir_visit_next_op_t(const char *));
        MOCK_METHOD1(on_leave_dir, dir_visit_next_op_t(const char *));
    };

    virtual void SetUp();
    virtual void TearDown();

    void expectFile(const char * file, dir_visit_next_op_t r);
    void expectEnter(const char * file, dir_visit_next_op_t r);
    void expectLeave(const char * file, dir_visit_next_op_t r);

    void do_search(int maxLevel);

    VisitCheckMock m_visitCheck;
};

#define CHECK_SEARCHDIR_RESULT() \
    ::testing::Mock::VerifyAndClear(&m_visitCheck);

#endif
