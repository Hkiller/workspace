#include "cpe/dr/dr_metalib_xml.h"
#include "cpe/tl/tl_manage.h"
#include "EvtTest.hpp"

gd_evt_t EvtTest::createEvt(const char * typeName, size_t carry_size, ssize_t data_capacity) {
    gd_evg_mgr_set_carry_info(t_evt_mgr(), NULL, carry_size);
    return gd_evt_create(t_evt_mgr(), typeName, data_capacity, t_em());
}
