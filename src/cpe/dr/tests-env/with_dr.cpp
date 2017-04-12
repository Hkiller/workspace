#include "cpe/pal/pal_platform.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"

namespace cpe { namespace dr { namespace testenv {

LPDRMETALIB with_dr::t_create_metalib(const char * xml) {
    error_monitor_t em = NULL;

    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    struct mem_buffer buffer;
    mem_buffer_init(&buffer, 0);

    int rv = dr_create_lib_from_xml_ex(&buffer, xml, strlen(xml), 0, em);
    EXPECT_TRUE(rv == 0)<< "create meta error";
    if (rv != 0) {
        mem_buffer_clear(&buffer);
        return NULL;
    }

    LPDRMETALIB metalib =
        (LPDRMETALIB)t_tmp_memdup(mem_buffer_make_continuous(&buffer, 0), mem_buffer_size(&buffer));
    EXPECT_TRUE(metalib);

    mem_buffer_clear(&buffer);

    return metalib;
}

const char * with_dr::t_dump_metalib_xml(LPDRMETALIB metalib) {
    error_monitor_t em = NULL;

    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    struct mem_buffer buffer;
    mem_buffer_init(&buffer, t_tmp_allocrator());

    char * r = dr_save_lib_to_xml_buf(&buffer, metalib, em);
    EXPECT_TRUE(r);
    return r;
}

}}}
