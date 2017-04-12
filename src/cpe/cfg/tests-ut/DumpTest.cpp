#include "DumpTest.hpp"

const char *
DumpTest::dump(const char * input) {
    return t_cfg_dump(t_cfg_parse(input));
}

const char *
DumpTest::dump_inline(const char * input) {
    return t_cfg_dump_inline(t_cfg_parse(input));
}
