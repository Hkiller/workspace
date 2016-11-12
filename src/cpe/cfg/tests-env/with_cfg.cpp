#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"

namespace cpe { namespace cfg { namespace testenv {

cfg_t with_cfg::t_cfg_create(void) {
    cfg_t cfg = cfg_create(t_tmp_allocrator());
    EXPECT_TRUE(cfg) << "t_cfg_create: create cfg fail!";
    return cfg;
}

cfg_t with_cfg::t_cfg_parse(const char * input) {
    if (input == 0) return 0;

    cfg_t cfg = cfg_create(t_tmp_allocrator());
    EXPECT_TRUE(cfg) << "t_cfg_parse: create cfg fail!";
    if (cfg == 0) return 0;

    read_stream_mem inputStream = CPE_READ_STREAM_MEM_INITIALIZER(input, strlen(input));
    EXPECT_EQ(
        0,
        cfg_yaml_read_with_name(cfg, "", (read_stream_t)&inputStream, cfg_merge_use_new, 0))
        << "parse cfg fail!\ninput:\n" << input;

    return cfg_struct_find_cfg(cfg, "");
}

void with_cfg::t_cfg_read(cfg_t cfg, const char * input) {
    read_stream_mem inputStream = CPE_READ_STREAM_MEM_INITIALIZER(input, strlen(input));

    error_monitor_t em = NULL;
    utils::testenv::with_em * with_em = tryEnvOf<utils::testenv::with_em>();
    if (with_em) {
        em = with_em->t_em();
    }

    EXPECT_EQ(
        0,
        cfg_yaml_read(cfg, (read_stream_t)&inputStream, cfg_replace, em));
}

const char * with_cfg::t_cfg_dump(cfg_t cfg, int ident, int level_ident) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, t_tmp_allocrator());

    return cfg_dump(cfg, &buffer, ident, level_ident);
}

const char * with_cfg::t_cfg_dump_inline(cfg_t cfg) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, t_tmp_allocrator());

    return cfg_dump_inline(cfg, &buffer);
}

}}}
 
