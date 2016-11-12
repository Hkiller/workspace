#include <sstream>
#include "SearchDirTest.hpp"

#ifndef TARGET_IPHONE_SIMULATOR

TEST_F(SearchDirTest, file_go) {
    expectFile("b.txt", dir_visit_next_go);
    expectFile("a.txt", dir_visit_next_go);

    t_write_to_file("a.txt", "");
    t_write_to_file("b.txt", "");

    do_search(-1);

    CHECK_SEARCHDIR_RESULT();
}

TEST_F(SearchDirTest, file_ignore) {
    expectFile("b.txt", dir_visit_next_go);
    expectFile("a.txt", dir_visit_next_ignore);

    t_write_to_file("a.txt", "");
    t_write_to_file("b.txt", "");

    do_search(-1);

    CHECK_SEARCHDIR_RESULT();
}

TEST_F(SearchDirTest, file_exit) {
    EXPECT_CALL(
        m_visitCheck
        , on_file(
            ::testing::AnyOf(
                ::testing::StrEq("a.txt"),
                ::testing::StrEq("b.txt"))))
        .WillOnce(::testing::Return(dir_visit_next_exit));

    t_write_to_file("a.txt", "");
    t_write_to_file("b.txt", "");

    do_search(-1);

    CHECK_SEARCHDIR_RESULT();
}

TEST_F(SearchDirTest, dir_basic) {
    expectEnter("a", dir_visit_next_go);
    expectFile("a.txt", dir_visit_next_go);
    expectLeave("a", dir_visit_next_go);

    t_dir_make("a");
    t_write_to_file("a/a.txt", "");

    do_search(-1);

    CHECK_SEARCHDIR_RESULT();
}

TEST_F(SearchDirTest, dir_enter_exit) {
    expectEnter("a", dir_visit_next_exit);

    t_dir_make("a");
    t_write_to_file("a/a.txt", "");

    do_search(-1);

    CHECK_SEARCHDIR_RESULT();
}

TEST_F(SearchDirTest, dir_enter_ignore) {
    expectEnter("a", dir_visit_next_ignore);
    expectLeave("a", dir_visit_next_go);

    t_dir_make("a");
    t_write_to_file("a/a.txt", "");

    do_search(-1);

    CHECK_SEARCHDIR_RESULT();
}

TEST_F(SearchDirTest, max_level_0) {
    t_write_to_file("a.txt", "");
    do_search(0);
    CHECK_SEARCHDIR_RESULT();
}

TEST_F(SearchDirTest, max_level_1) {
    expectEnter("a", dir_visit_next_go);
    expectLeave("a", dir_visit_next_go);
    t_dir_make("a");
    t_write_to_file("a/a.txt", "");
    do_search(1);
    CHECK_SEARCHDIR_RESULT();
}

TEST_F(SearchDirTest, dir_multi_level) {
    expectEnter("a", dir_visit_next_go);
    expectEnter("b", dir_visit_next_go);
    expectEnter("c", dir_visit_next_go);
    expectLeave("c", dir_visit_next_go);
    expectLeave("b", dir_visit_next_go);
    expectLeave("a", dir_visit_next_go);

    t_dir_make("a/b/c");

    do_search(-1);

    CHECK_SEARCHDIR_RESULT();
}

TEST_F(SearchDirTest, dir_alloc_count) {
    expectEnter("a", dir_visit_next_go);
    expectEnter("b", dir_visit_next_go);
    expectEnter("c", dir_visit_next_go);
    expectLeave("c", dir_visit_next_go);
    expectLeave("b", dir_visit_next_go);
    expectLeave("a", dir_visit_next_go);

    t_dir_make("a/b/c");

    do_search(-1);

    CHECK_SEARCHDIR_RESULT();

    EXPECT_EQ(1, t_alloc_count());
}

#endif
