#include "BpgPkgTest.hpp"

class BpgPkgDataTest : public BpgPkgTest {
public:
    BpgPkgDataTest() : m_pkg(NULL) {
    }

    virtual void SetUp() {
        BpgPkgTest::SetUp();

        m_pkg = t_bpg_pkg_create();
    }

    virtual void TearDown() {
        if (m_pkg) {
            dp_req_free(m_pkg);
            m_pkg = NULL;
        }

        BpgPkgTest::TearDown();
    }

    dp_req_t m_pkg;
};

TEST_F(BpgPkgDataTest, cmd_no_meta) {
    bpg_pkg_set_cmd(m_pkg, 2);
    EXPECT_EQ((uint32_t)2, bpg_pkg_cmd(m_pkg));
}
