#include "WriteToXmlTest.hpp"

void WriteToXmlTest::SetUp() {
    Base::SetUp();
}

void WriteToXmlTest::TearDown() {
    Base::TearDown();
}

const char *
WriteToXmlTest::writeToXml(const char * def) {
    struct mem_buffer input_buffer;
    struct mem_buffer output_buffer;

    mem_buffer_init(&input_buffer, NULL);
    mem_buffer_init(&output_buffer, NULL);

    t_elist_clear();

    int r = dr_create_lib_from_xml_ex(&input_buffer, def, strlen(def), 0, NULL);
    EXPECT_EQ(0, r);

    LPDRMETALIB  metaLib = (LPDRMETALIB)mem_buffer_make_continuous(&input_buffer, 0);

    const char * result =
        t_tmp_strdup(
            dr_save_lib_to_xml_buf(&output_buffer, metaLib, NULL));

    mem_buffer_clear(&input_buffer);
    mem_buffer_clear(&output_buffer);

    return result;
}
