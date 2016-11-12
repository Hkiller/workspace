#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "ModifyTest.hpp"

ModifyTest::ModifyTest() {
}

const char * ModifyTest::modify(const char * cfg, const char * modify) {
    cfg_t c = t_cfg_parse(cfg);
    EXPECT_TRUE(c);
    if (c == NULL) return "parse cfg fail!";

    cfg_t m = t_cfg_parse(modify);
    EXPECT_TRUE(m);
    if (m == NULL) return "parse modify fail!";

    int r = cfg_apply_modify(c, m, t_em());
    if (r != 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d", r);
        return t_tmp_strdup(buf);
    }

    return t_cfg_dump(c);
}

