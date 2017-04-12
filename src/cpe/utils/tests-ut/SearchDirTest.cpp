#include "SearchDirTest.hpp"

#ifndef TARGET_IPHONE_SIMULATOR

void SearchDirTest::SetUp() {
    Base::SetUp();

    ::testing::DefaultValue<dir_visit_next_op_t>::Set(dir_visit_next_go);
}

void SearchDirTest::TearDown() {
    ::testing::DefaultValue<dir_visit_next_op_t>::Clear();
    Base::TearDown();
}

static dir_visit_next_op_t
on_dir_enter(const char * full, const char * base, void * ctx) {
    SearchDirTest * t = (SearchDirTest *)ctx;
    return t->m_visitCheck.on_enter_dir(base);
}

static dir_visit_next_op_t
on_dir_leave(const char * full, const char * base, void * ctx) {
    SearchDirTest * t = (SearchDirTest *)ctx;
    return t->m_visitCheck.on_leave_dir(base);
}

static dir_visit_next_op_t
on_file(const char * full, const char * base, void * ctx) {
    SearchDirTest * t = (SearchDirTest *)ctx;
    return t->m_visitCheck.on_file(base);
}

void
SearchDirTest::do_search(int maxLevel) {
    struct dir_visitor visitor;
    visitor.on_dir_enter = on_dir_enter;
    visitor.on_dir_leave = on_dir_leave;
    visitor.on_file = on_file;
    dir_search(&visitor, this, t_dir_base(), maxLevel, t_em(), t_allocrator());
}

void SearchDirTest::expectFile(const char * file, dir_visit_next_op_t r) {
    EXPECT_CALL(m_visitCheck, on_file(::testing::StrEq(file)))
        .WillOnce(::testing::Return(r));
}

void SearchDirTest::expectEnter(const char * file, dir_visit_next_op_t r) {
    EXPECT_CALL(m_visitCheck, on_enter_dir(::testing::StrEq(file)))
        .WillOnce(::testing::Return(r));
}

void SearchDirTest::expectLeave(const char * file, dir_visit_next_op_t r) {
    EXPECT_CALL(m_visitCheck, on_leave_dir(::testing::StrEq(file)))
        .WillOnce(::testing::Return(r));
}

#endif
