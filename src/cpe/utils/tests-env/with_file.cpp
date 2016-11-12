#include <stdlib.h>
#include "cpe/utils/tests-env/with_file.hpp"

namespace utils { namespace testenv {

void with_file::SetUp() {
    char * name_buf = t_tmp_strdup("/tmp/gtest.XXXXXX");
    EXPECT_TRUE(name_buf);
    m_path_base = mkdtemp(name_buf);
}

void with_file::TearDown() {
    if (m_path_base) {
        dir_rm_recursion(m_path_base, NULL, t_tmp_allocrator());
        mem_free(t_tmp_allocrator(), m_path_base);
        m_path_base = NULL;
    }
}

const char * 
with_file::t_dir_base(void) {
    return m_path_base;
}

char *
with_file::t_path_make(const char * subpath) {
    int len = strlen(subpath);
    int baselen = strlen(m_path_base);

    char * buf = (char*)t_tmp_alloc(baselen + len + 1 + 1);
    memcpy(buf, m_path_base, baselen);
    memcpy(buf + baselen, "/", 1);
    memcpy(buf + baselen + 1, subpath, len);
    buf[len + baselen + 1] = 0;

    return buf;
}

char * with_file::t_file_to_str(const char * sub) {
    CPE_DEF_ERROR_MONITOR(tem, cpe_error_log_to_consol, NULL);

    char * path = t_path_make(sub);

    if (!file_exist(path, &tem)) return NULL;

    FILE * fp = file_stream_open(path, "r", &tem);
    if (fp == NULL) return NULL;

    int size  = file_stream_size(fp, &tem);
    if (size < 0) return NULL;

    char * buf = (char*)t_tmp_alloc(size + 1);

    int loadSize = file_stream_load_to_buf(buf, size, fp, &tem);
    if (loadSize < 0 || loadSize > size) return NULL;

    buf[size] = 0;

    return buf;
}

void with_file::t_write_to_file(const char * subname, const char * data) {
    CPE_DEF_ERROR_MONITOR(tem, cpe_error_log_to_consol, NULL);
    EXPECT_LE(0, file_write_from_str( t_path_make(subname), data, &tem));
}

void with_file::t_dir_make(const char * subpath) {
    CPE_DEF_ERROR_MONITOR(tem, cpe_error_log_to_consol, NULL);
    EXPECT_EQ(
        0,
        dir_mk_recursion(
            t_path_make(subpath),
            DIR_DEFAULT_MODE,
            &tem,
            t_tmp_allocrator()));
}

}}

