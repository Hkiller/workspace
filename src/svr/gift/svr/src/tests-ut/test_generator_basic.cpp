#include "GeneratorTest.hpp"

class GeneratorBasicTest : public GeneratorTest {
    virtual void SetUp() {
        GeneratorTest::SetUp();

        t_generate_mgr_init(
            "<metalib tagsetversion='1' version='1'>"
            "    <struct name='Data' version='1'>"
            "        <entry name='gift_type' type='uint16' id='1'/>"
            "    </struct>"
            "</metalib>"
            );

        t_generator_create(
            "test", 
            "{ prefix-len: 2, prefix-scope: num, blocks: [ { len: 4, scope: num } ] }");
    }
};

TEST_F(GeneratorBasicTest, empty) {
    ASSERT_PREFIX_MATCH(select_prefix(), m_generator);
}
