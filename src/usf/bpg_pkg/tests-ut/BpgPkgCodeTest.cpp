#include "BpgPkgCodeTest.hpp"

BpgPkgCodeTest::BpgPkgCodeTest()
    : m_pkg(NULL)
    , m_encode_pkg(NULL)
{
}

void BpgPkgCodeTest::SetUp() {
    BpgPkgTest::SetUp();

    t_dr_cvt_install_pbuf();
    t_dr_cvt_install_pbuf_len();
}

void BpgPkgCodeTest::TearDown() {
    if (m_pkg) {
        dp_req_free(m_pkg);
        m_pkg = NULL;
    }

    BpgPkgTest::TearDown();
}

void BpgPkgCodeTest::set_model(const char * model, uint8_t encode_align, const char * module_name) {
    t_bpg_pkg_manage_set_model(model, encode_align, module_name);
    t_bpg_pkg_manage_set_cvt("pbuf", "pbuf-len", module_name);
}

void BpgPkgCodeTest::add_cmd(uint32_t cmd, const char * meta, const char * mgr_name) {
    t_bpg_pkg_manage_add_cmd(cmd, meta, mgr_name);
}

dr_cvt_result_t BpgPkgCodeTest::encode(const char * data, const char * mgr_name) {
    dp_req_t body = t_bpg_pkg_build(data, mgr_name);
    EXPECT_TRUE(body) << "build pkg fail!";
    if (body == NULL) return dr_cvt_result_error;

    dr_cvt_result_t r = encode(body);

    dp_req_free(body);

    return r;
}

dr_cvt_result_t BpgPkgCodeTest::encode(dp_req_t body) {
    m_encode_pkg = t_bpg_pkg_dump(body);
    m_buf_size = sizeof(m_buf);
    return bpg_pkg_encode(body, m_buf, &m_buf_size, t_em(), 1);
}

const char * BpgPkgCodeTest::decode(const char * mgr_name) {
    dp_req_t pkg = t_bpg_pkg_create(1024, mgr_name);
    EXPECT_TRUE(pkg) << "build pkg fail!";
    if (pkg == NULL) return NULL;

    size_t size = m_buf_size;
    dr_cvt_result_t cvt_result = bpg_pkg_decode(pkg, m_buf, &size, t_em(), 1);
    EXPECT_EQ(size, m_buf_size) << "encode size and decode use sizie mismatch!";

    const char *  r = "";
    switch(cvt_result) {
    case dr_cvt_result_success:
        r = t_bpg_pkg_dump(pkg);
        break;
    case dr_cvt_result_error:
        r = "error";
        break;
    case dr_cvt_result_not_enough_input:
        r = "not error input";
        break;
    case dr_cvt_result_not_enough_output:
        r = "not enouth output";
        break;
    default:
        r = "unknown";
    }

    dp_req_free(pkg);

    return r;
}

