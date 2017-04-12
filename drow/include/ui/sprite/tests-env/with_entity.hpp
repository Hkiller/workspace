#ifndef UI_SPRITE_TESTENV_WITH_ENTITY_H
#define UI_SPRITE_TESTENV_WITH_ENTITY_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../ui_sprite_entity.h"
#include "../ui_sprite_entity_attr.h"
#include "../ui_sprite_entity_calc.h"

namespace ui { namespace sprite { namespace testenv {

class with_entity : public ::testenv::env<> {
public:
    with_entity();

    void SetUp();
    void TearDown();

    ui_sprite_entity_t t_s_entity(void);

    /*event operations*/
    void t_s_entity_send_event(const char * event);
    void t_s_entity_send_event(LPDRMETA meta, void const * data, size_t data_size);

    void t_s_entity_debug(uint8_t debug);

    /*calc*/
    int8_t t_s_entity_calc_bool(const char * def, dr_data_source_t data_source = NULL);
    int64_t t_s_entity_calc_int64(const char * def, dr_data_source_t data_source = NULL);
    double t_s_entity_calc_double(const char * def, dr_data_source_t data_source = NULL);
    const char * t_s_entity_calc_str(const char * def, dr_data_source_t data_source = NULL);

private:
    uint32_t m_entity_id;
};

}}}

#endif
