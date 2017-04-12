#ifndef USF_BPG_PKG_CODE_TEST_BPGPKGTEST_H
#define USF_BPG_PKG_CODE_TEST_BPGPKGTEST_H
#include "BpgPkgTest.hpp"

class BpgPkgCodeTest : public BpgPkgTest {
public:
    BpgPkgCodeTest();
    virtual void SetUp();
    virtual void TearDown();

    void set_model(const char * model, uint8_t encode_align = 0, const char * mgr_name = NULL);
    void add_cmd(uint32_t cmd, const char * meta, const char * mgr_name = NULL);

    dr_cvt_result_t encode(dp_req_t body);
    dr_cvt_result_t encode(const char * data, const char * mgr_name = NULL);
    const char * decode(const char * mgr_name = NULL);

    dp_req_t m_pkg;
    const char * m_encode_pkg;
    char m_buf[1024];
    size_t m_buf_size;
};

#endif
