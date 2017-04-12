#include "NodeTest.hpp"

TEST_F(NodeTest, read_int8_basic) {
    install(
        "a:\n"
        "    b: 1\n");

    int8_t v = root()["a.b"];
    EXPECT_EQ((int8_t)1, v);
}

TEST_F(NodeTest, read_int8_no_path) {
    EXPECT_THROW(
        (int8_t)root()["a.b"],
        ::std::exception);
}

TEST_F(NodeTest, read_error) {
    install(
        "a:\n"
        "    b: abc\n");

    EXPECT_THROW(
        (int8_t)root()["a.b"],
        ::std::exception);
}

TEST_F(NodeTest, read_int8_with_dft_basic) {
    install(
        "a:\n"
        "    b: 1\n");

    Cpe::Cfg::Node & n = root();
    int8_t v = n["a.b"].dft(12);
    EXPECT_EQ((int8_t)1, v);
}
