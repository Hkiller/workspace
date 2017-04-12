#include "FileIdGeneratorTest.hpp"

TEST_F(FileIdGeneratorTest, generate_first) {
    gd_id_t r;

    EXPECT_EQ(0, gd_id_generator_generate(&r, id_generator(), "test"));
    EXPECT_EQ((gd_id_t)1, r);
}

TEST_F(FileIdGeneratorTest, generate_to_next_range) {
    gd_id_t r;

    gd_id_file_generator_set_once_alloc_size(id_file_generator(), 12);

    for(int i = 0; i < 100; ++i) {
        EXPECT_EQ(0, gd_id_generator_generate(&r, id_generator(), "test"));
        EXPECT_EQ((gd_id_t)(i + 1), r);
    }
}

TEST_F(FileIdGeneratorTest, with_file_generate_first) {
    gd_id_t r;

    set_load_from_dir();

    EXPECT_EQ(0, gd_id_generator_generate(&r, id_generator(), "test"));
    EXPECT_EQ((gd_id_t)1, r);
}

TEST_F(FileIdGeneratorTest, with_file_generate_to_next_range) {
    gd_id_t r;

    set_load_from_dir();
    gd_id_file_generator_set_once_alloc_size(id_file_generator(), 12);

    for(int i = 0; i < 100; ++i) {
        EXPECT_EQ(0, gd_id_generator_generate(&r, id_generator(), "test"));
        EXPECT_EQ((gd_id_t)(i + 1), r);
    }
}
