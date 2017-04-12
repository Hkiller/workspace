#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_build.h"
#include "GenerateTest.hpp"

GenerateTest::GenerateTest() {
    m_ctx.m_builder = 0;
    m_ctx.m_metalib = 0;
    m_ctx.m_em = 0;
}

void GenerateTest::SetUp() {
    Base::SetUp();

    m_ctx.m_builder = dr_metalib_builder_create(t_tmp_allocrator(), t_em());
    ASSERT_TRUE(m_ctx.m_builder);
    m_ctx.m_em = t_em();
}

void GenerateTest::TearDown() {
    Base::TearDown();

    if (m_ctx.m_builder) {
        dr_metalib_builder_free(m_ctx.m_builder);
        m_ctx.m_builder = NULL;
    }
}

dr_metalib_source_t GenerateTest::add_buffer(const char * name, const char * data) {
    EXPECT_TRUE(NULL == m_ctx.m_metalib);

    if (m_ctx.m_metalib) {
        return NULL;
    }
    else {
        return dr_metalib_builder_add_buf(m_ctx.m_builder, name, dr_metalib_source_format_xml, data);
    }
}

dr_metalib_source_t GenerateTest::source(const char * name) {
    return dr_metalib_source_find(m_ctx.m_builder, name);
}

void GenerateTest::prepare_ctx(void) {
    if (m_ctx.m_metalib) return;

    mem_buffer buffer;
    mem_buffer_init(&buffer, t_tmp_allocrator());

    dr_metalib_builder_analize(m_ctx.m_builder);

    int r = dr_inbuild_build_lib(
        &buffer,
        dr_metalib_bilder_lib(m_ctx.m_builder),
        t_em());
    EXPECT_EQ(0, r) << "build meta lib fail!";
    if (r == 0) {
        m_ctx.m_metalib = (LPDRMETALIB)t_tmp_memdup(mem_buffer_make_continuous(&buffer, 0), mem_buffer_size(&buffer));
    }

    mem_buffer_clear(&buffer);
}

const char * GenerateTest::generate_h(const char * name) {
    prepare_ctx();
    if (m_ctx.m_metalib == 0) return NULL;

    mem_buffer buffer;
    mem_buffer_init(&buffer, 0);

    write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);

    EXPECT_EQ(0, cpe_dr_generate_h((write_stream_t)&stream, source(name), 0, &m_ctx));
    stream_putc((write_stream_t)&stream, 0);
    
    char * r = t_tmp_strdup((char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);

    return r;
}

const char * GenerateTest::generate_lib_c(const char * arg_name) {
    prepare_ctx();
    if (m_ctx.m_metalib == 0) return NULL;

    mem_buffer buffer;
    mem_buffer_init(&buffer, 0);

    write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);

    EXPECT_EQ(0, cpe_dr_generate_lib_c((write_stream_t)&stream, arg_name, &m_ctx));
    stream_putc((write_stream_t)&stream, 0);
    
    char * r = t_tmp_strdup((char *)mem_buffer_make_continuous(&buffer, 0));
    mem_buffer_clear(&buffer);

    return r;
}
