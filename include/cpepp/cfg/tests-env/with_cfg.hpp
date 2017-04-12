#ifndef CPEPP_CFG_TESTENV_WITH_CFG_H
#define CPEPP_CFG_TESTENV_WITH_CFG_H
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "../Node.hpp"

namespace Cpe { namespace Cfg { namespace testenv {

class with_cfg : public cpe::cfg::testenv::with_cfg {
public:
    Node & t_cfg_parse_ex(const char * input) {
        return Node::_cast(t_cfg_parse(input));
    }
};

}}}

#endif
