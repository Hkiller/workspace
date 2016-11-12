#include "cpe/utils/stream_file.h"
#include "FileTest.hpp"

#ifndef TARGET_IPHONE_SIMULATOR

class FileStreamTest : public FileTest {
public:
    FileStreamTest() : m_fp(NULL) {
    }

    virtual void SetUp() {
        FileTest::SetUp();
    }

    virtual void TearDown() {
        if (m_fp) {
            CPE_DEF_ERROR_MONITOR(tem, cpe_error_log_to_consol, NULL);
            file_stream_close(m_fp, &tem);
            m_fp = NULL;
        }

        FileTest::TearDown();
    }

    FILE * open(const char * name, const char * mode) {
        CPE_DEF_ERROR_MONITOR(tem, cpe_error_log_to_consol, NULL);

        if (m_fp) {
            file_stream_close(m_fp, &tem);
            m_fp = NULL;
        }

        m_fp = file_stream_open(t_path_make(name), mode, &tem);
        EXPECT_TRUE(m_fp);
        return m_fp;
    }

    FILE * m_fp;
};

TEST_F(FileStreamTest, read_basic) {
    t_write_to_file("a.txt", "abcd");

    struct read_stream_file stream;
    read_stream_file_init(&stream, open("a.txt", "r"), t_em());

    char buf[256];

    EXPECT_EQ(
        4,
        stream_read((read_stream_t)&stream, buf, 256));
    buf[4] = 0;
    EXPECT_STREQ("abcd", buf);

    EXPECT_EQ(
        0,
        stream_read((read_stream_t)&stream, buf, 256));
}

TEST_F(FileStreamTest, write_basic) {
    struct write_stream_file stream;
    write_stream_file_init(&stream, open("a.txt", "w"), t_em());

    EXPECT_EQ(
        4,
        stream_write((write_stream_t)&stream, "abcd", 4));
    
    stream_flush((write_stream_t)&stream);

    EXPECT_STREQ("abcd", t_file_to_str("a.txt"));
}

#endif
