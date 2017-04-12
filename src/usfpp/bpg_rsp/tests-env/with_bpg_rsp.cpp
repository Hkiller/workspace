#include "usfpp/bpg_rsp/tests-env/with_bpg_rsp.hpp"

namespace Usf { namespace Bpg { namespace testenv {

with_bpg_rsp::with_bpg_rsp() {
}

void with_bpg_rsp::SetUp() {
    usf::bpg::testenv::with_bpg_rsp::SetUp();
}

void with_bpg_rsp::TearDown() {
    usf::bpg::testenv::with_bpg_rsp::TearDown();
}

}}}

